// ============================================================
// Hexapod Spider Robot - Controller Implementation
// ============================================================
#include "hexapod_controller.h"

// ============================================================
// HexapodLeg Constructor
// ============================================================
HexapodLeg::HexapodLeg(Adafruit_PWMServoDriver* pwm, float L1, float L2, float L3,
                       JointConfig coxa, JointConfig femur, JointConfig tibia)
    : _ikSolver(L1, L2, L3) {
    _pwm = pwm;
    _coxa = coxa;
    _femur = femur;
    _tibia = tibia;
}

// ============================================================
// Set Foot Position in Cartesian Space
// ============================================================
void HexapodLeg::setCartesian(float x, float y, float z) {
    ServoAngles angles = _ikSolver.calculate(x, y, z);
    setJointAngles(angles.coxa, angles.femur, angles.tibia);
}

// ============================================================
// Set Joint Angles Directly
// ============================================================
void HexapodLeg::setJointAngles(float coxa, float femur, float tibia) {
    _pwm->setPWM(_coxa.channel, 0, angleToPulse(coxa, _coxa));
    _pwm->setPWM(_femur.channel, 0, angleToPulse(femur, _femur));
    _pwm->setPWM(_tibia.channel, 0, angleToPulse(tibia, _tibia));
}

// ============================================================
// Convert Angle (0-180) to PWM Pulse
// ============================================================
uint16_t HexapodLeg::angleToPulse(float angle, const JointConfig& cfg) {
    // Clamp angle
    float clamped = constrain(angle, 0.0f, 180.0f);
    
    // PWM range: 102 (0.5ms) to 512 (2.5ms) for 50Hz
    const uint16_t PWM_MIN = 102;
    const uint16_t PWM_MAX = 512;
    
    float pulse;
    if (cfg.isInverted) {
        pulse = PWM_MAX - (clamped / 180.0f) * (PWM_MAX - PWM_MIN);
    } else {
        pulse = PWM_MIN + (clamped / 180.0f) * (PWM_MAX - PWM_MIN);
    }
    
    return (uint16_t)(round(pulse) + cfg.offset);
}

// ============================================================
// Go to Home Position (all joints at 90 degrees)
// ============================================================
void HexapodLeg::goHome() {
    setJointAngles(90.0f, 90.0f, 90.0f);
}

// ============================================================
// HexapodController Constructor
// ============================================================
HexapodController::HexapodController()
    : _pwm1(0x40), _pwm2(0x41),
      _globalIK(COXA_LENGTH, FEMUR_LENGTH, TIBIA_LENGTH),
      legs{
          // Front Left  (PCA9685 #1)
          HexapodLeg(&_pwm1, COXA_LENGTH, FEMUR_LENGTH, TIBIA_LENGTH,
                     {0, 15, false}, {4, 10, true}, {8, 20, false}),
          // Middle Right (PCA9685 #1)
          HexapodLeg(&_pwm1, COXA_LENGTH, FEMUR_LENGTH, TIBIA_LENGTH,
                     {1, -10, false}, {5, -20, true}, {9, 50, false}),
          // Back Left   (PCA9685 #1)
          HexapodLeg(&_pwm1, COXA_LENGTH, FEMUR_LENGTH, TIBIA_LENGTH,
                     {2, 40, false}, {6, -30, true}, {10, 80, false}),
          // Front Right (PCA9685 #2)
          HexapodLeg(&_pwm2, COXA_LENGTH, FEMUR_LENGTH, TIBIA_LENGTH,
                     {0, -10, false}, {4, 0, true}, {8, 50, false}),
          // Middle Left (PCA9685 #2)
          HexapodLeg(&_pwm2, COXA_LENGTH, FEMUR_LENGTH, TIBIA_LENGTH,
                     {1, 5, false}, {5, 30, true}, {9, -5, false}),
          // Back Right  (PCA9685 #2)
          HexapodLeg(&_pwm2, COXA_LENGTH, FEMUR_LENGTH, TIBIA_LENGTH,
                     {2, 40, false}, {6, -40, true}, {10, -60, false})
      } {
    _isWalking = false;
    _walkDirection = 0.0f;
    _turnDirection = 0.0f;
    _walkStartTime = 0;
    _currentStanceZ = BASE_STANCE_Z_VAL;
    _currentYawXOffset = 0.0f;
    _heightLevel = 0;
    _yawLevel = 0;
    
    for (int i = 0; i < 6; i++) {
        _legPosX[i] = 0;
        _legPosY[i] = STANCE_Y_VAL;
        _legPosZ[i] = _currentStanceZ;
    }
}

// ============================================================
// Initialize Controller
// ============================================================
void HexapodController::begin() {
    _pwm1.begin();
    _pwm1.setPWMFreq(SERVO_FREQ);
    _pwm2.begin();
    _pwm2.setPWMFreq(SERVO_FREQ);
    
    Serial.println("🦿 Initializing Hexapod...");
    
    // Soft-start: slowly move to home position
    for (int i = 0; i < 6; i++) {
        legs[i].goHome();
        delay(500);
    }
    
    home();
    Serial.println("✅ Hexapod Initialized");
}

// ============================================================
// Bezier Curve for Smooth Foot Lift
// ============================================================
inline float HexapodController::calculateBezierLift(float peakHeight, float t) {
    // Quadratic Bezier: returns lift height at phase t (0-1)
    return 4.0f * (1.0f - t) * t * peakHeight;
}

// ============================================================
// Main Update Loop - Called Every Cycle
// ============================================================
void HexapodController::update() {
    if (!_isWalking) return;
    
    unsigned long elapsed = millis() - _walkStartTime;
    float stridePhase = fmod((float)elapsed / CYCLE_TIME, 1.0f);
    
    // ============================================================
    // Calculate Tripod A (FL, MR, BL) and Tripod B (FR, ML, BR)
    // ============================================================
    float xA, zA, xB, zB;
    
    // Tripod A: Swing phase (0-0.5), Stance phase (0.5-1.0)
    if (stridePhase < 0.5f) {
        float t = stridePhase / 0.5f;  // 0 to 1
        xA = (-STEP_LENGTH/2.0f + (t * STEP_LENGTH)) * _walkDirection;
        zA = _currentStanceZ + calculateBezierLift(STEP_HEIGHT, t);
    } else {
        float t = (stridePhase - 0.5f) / 0.5f;  // 0 to 1
        xA = (STEP_LENGTH/2.0f - (t * STEP_LENGTH)) * _walkDirection;
        zA = _currentStanceZ;
    }
    
    // Tripod B: Opposite phase (180 degrees offset)
    if (stridePhase < 0.5f) {
        float t = stridePhase / 0.5f;
        xB = (STEP_LENGTH/2.0f - (t * STEP_LENGTH)) * _walkDirection;
        zB = _currentStanceZ;
    } else {
        float t = (stridePhase - 0.5f) / 0.5f;
        xB = (-STEP_LENGTH/2.0f + (t * STEP_LENGTH)) * _walkDirection;
        zB = _currentStanceZ + calculateBezierLift(STEP_HEIGHT, t);
    }
    
    // ============================================================
    // Apply Turning Mirror Effect
    // ============================================================
    float mirroredX_A = xA;
    float mirroredX_B = xB;
    
    if (_turnDirection == 1.0f) {
        // Turn Right: right legs move less, left legs move more
        mirroredX_A = xA;  // Left side normal
        mirroredX_B = -xB; // Right side mirrored
    } else if (_turnDirection == -1.0f) {
        // Turn Left: left legs move less, right legs move more
        mirroredX_A = -xA; // Left side mirrored
        mirroredX_B = xB;  // Right side normal
    }
    
    // ============================================================
    // Update Each Leg
    // ============================================================
    float targetX, targetZ;
    bool isGroupA;
    
    for (int i = 0; i < 6; i++) {
        // Determine which tripod group this leg belongs to
        isGroupA = (i == FRONT_LEFT || i == MIDDLE_RIGHT || i == BACK_LEFT);
        
        targetX = isGroupA ? mirroredX_A : mirroredX_B;
        targetZ = isGroupA ? zA : zB;
        
        // Apply body yaw offset
        float finalX = targetX + _currentYawXOffset;
        
        // Apply terrain safety (prevent legs from going through floor)
        if (targetZ > _currentStanceZ) targetZ = _currentStanceZ;
        
        // Update leg position
        legs[i].setCartesian(finalX, STANCE_Y_VAL, targetZ);
        
        // Store for smooth transitions
        _legPosX[i] = finalX;
        _legPosY[i] = STANCE_Y_VAL;
        _legPosZ[i] = targetZ;
    }
}

// ============================================================
// Movement Commands
// ============================================================
void HexapodController::forward() {
    _isWalking = true;
    _walkDirection = 1.0f;
    _turnDirection = 0.0f;
    _walkStartTime = millis();
    Serial.println("🚀 Moving Forward");
}

void HexapodController::backward() {
    _isWalking = true;
    _walkDirection = -1.0f;
    _turnDirection = 0.0f;
    _walkStartTime = millis();
    Serial.println("🔙 Moving Backward");
}

void HexapodController::turnLeft() {
    _isWalking = true;
    _walkDirection = 1.0f;
    _turnDirection = -1.0f;
    _walkStartTime = millis();
    Serial.println("⬅️ Turning Left");
}

void HexapodController::turnRight() {
    _isWalking = true;
    _walkDirection = 1.0f;
    _turnDirection = 1.0f;
    _walkStartTime = millis();
    Serial.println("➡️ Turning Right");
}

void HexapodController::stop() {
    _isWalking = false;
    _walkDirection = 0.0f;
    _turnDirection = 0.0f;
    Serial.println("⏹️ Stopped");
}

void HexapodController::home() {
    stop();
    for (int i = 0; i < 6; i++) {
        legs[i].goHome();
    }
    Serial.println("
