#include "../include/helperfunctions.h"

//take in a vecor of motors, and set their speed to a value
void setMotors(std::vector<pros::Motor> & motors, double speed)
{
  for(auto motor : motors)
  {
    motor = speed;
  }
}

//take in a vector of motors, and set their brake type to a given type
void setBrakes(std::vector<pros::Motor> & motors,  pros::motor_brake_mode_e_t brakeType)
{
  for(auto motor: motors)   
  {
    motor.set_brake_mode(brakeType);
  }
}

//tqke in a vector of motors, and call the move relative function for all of them with a given distance and speed
void setMotorsRelative(std::vector<pros::Motor> & motors, double distance, double speed)
{
  for(auto motor : motors)
  {
    motor.move_relative(distance, speed);
  }
}

//function used in autononomous to drive for a given distance at a given speed
void driveDist(double target, DIRECTION direction, int numCubes, double maxSpeed)
{
    double speed = maxSpeed;
    double endDistance = 0;
    double startDistance = 2 * ROTATION_MUL;
    double currDist = 0;
    double deccelDist = 2 * ROTATION_MUL;
    double averagePos = 0;
    double distPercent = 0;
    int stopCount = 0;

    gyro.reset();

    auto oldBrake = wheelMotorVector[0].get_brake_mode();
    setBrakes(wheelMotorVector,  pros::E_MOTOR_BRAKE_BRAKE );

    setDirection(direction);
    target *= ROTATION_MUL;
    deccelDist = distReq(maxSpeed, numCubes, direction);
    startDistance = distReq(maxSpeed, numCubes, direction);
    endDistance = target - deccelDist;

    if ( startDistance + deccelDist > target)
    {
        distPercent = target / (startDistance + deccelDist);
        maxSpeed *= distPercent;
        startDistance *= distPercent;
        endDistance *= distPercent;
    }

    if (maxSpeed > 10)
    {
        for (int i = 0; i < wheelMotorVector.size(); i++) //sets the motors to 0
        {
            wheelMotorVector[i].set_zero_position(0);
        }

        while (averagePos < startDistance && stopCount < STOP_AMOUNT)
        {
            averagePos = (wheelLeft1.get_position() + wheelRight1.get_position() +
                    wheelLeft3.get_position() + wheelRight3.get_position()) / 4;
                    speed = maxSpeed * (averagePos / startDistance) + 14;
            if(speed < 20)
                speed = 20;
            if (speed > maxSpeed)
            {
                speed = maxSpeed;
            }

            correctDist(leftWheelMotorVector, rightWheelMotorVector, averagePos, speed, gyro.get_value(), direction);
            pros::delay(20);
            if (wheelLeft1.get_actual_velocity() <= 5)
                stopCount++;
        }

        while (averagePos < endDistance && stopCount < STOP_AMOUNT)
        {
            averagePos = (wheelLeft1.get_position() + wheelRight1.get_position() +
            wheelLeft3.get_position() + wheelRight3.get_position()) / 4;

            correctDist(leftWheelMotorVector, rightWheelMotorVector, averagePos, speed, gyro.get_value(), direction);

            pros::delay(20);
            if (wheelLeft1.get_actual_velocity() <= 5)
                stopCount++;
        }

        endDistance = target - averagePos;
        while (averagePos < target && stopCount < STOP_AMOUNT)
        {
            averagePos = (wheelLeft1.get_position() + wheelRight1.get_position() +
            wheelLeft3.get_position() + wheelRight3.get_position()) / 4;
            currDist = target - averagePos;
            speed = maxSpeed * (currDist / endDistance) + 14;
            if(speed < 20)
                speed = 20;
            if (speed > maxSpeed)
            {
               speed = maxSpeed;
            }

            correctDist(leftWheelMotorVector, rightWheelMotorVector, averagePos, speed, gyro.get_value(), direction);
            pros::delay(20);
            if (wheelLeft1.get_actual_velocity() <= 5)
                stopCount++;
        }
    }
    else
    {
        speed = 10;

        while (averagePos < endDistance && stopCount < STOP_AMOUNT)
        {
            averagePos = (wheelLeft1.get_position() + wheelRight1.get_position() +
            wheelLeft3.get_position() + wheelRight3.get_position()) / 4;

            correctDist(leftWheelMotorVector, rightWheelMotorVector, averagePos, speed, gyro.get_value(), direction);

            pros::delay(20);
            if (wheelLeft1.get_actual_velocity() <= 5)
                stopCount++;
        }
    }
    //displayPosition();

    setMotors(wheelMotorVector, 0);
    setDirection(FORWARD);
    pros::delay(150);
    setBrakes(wheelMotorVector,  oldBrake );
}

void cubeRun(double target, int numCubes)
{
    double intakeSpeed = 120;
    setMotors(intakeMotors, intakeSpeed);
    driveDist(target, FORWARD, -2, 40);
    if (numCubes < 8)
        pros::delay(300);
    setMotors(intakeMotors, 0);
}

//a function that finds the best speed based on the distance of the wheels
void correctDist (std::vector<pros::Motor> leftMotors, std::vector<pros::Motor> rightMotors,
    double target, double speed, double gyroVal, DIRECTION direction)
{
    double leftValue = 0;
    double rightValue = 0;
    double adjustUp = 1.1;
    double adjustDown = 0.9;
    for(int i = 0; i < leftMotors.size(); i++)
    {
      leftValue += leftMotors[i].get_position();
      rightValue += rightMotors[i].get_position();
    }

    leftValue /= leftMotors.size();
    leftValue -= target;

    rightValue /= rightMotors.size();
    rightValue -= target;

    double leftSpeed = speed;
    double rightSpeed = speed;

    //unknown if actually works, can't really test
    if ( rightValue > 2)
    {
        leftSpeed *= adjustDown;
    }
    else if (rightValue < -2)
    {
        leftSpeed *= adjustUp;
    }


    if ( leftValue > 2)
    {
        leftSpeed *= adjustDown;
    }
    else if (leftValue < -2)
    {
        leftSpeed *= adjustUp;
    }


    if(direction == FORWARD)
    {
        if(gyroVal < -10)
        {
            rightSpeed *= 0.92;
        }
        else if(gyroVal > 10)
        {
            leftSpeed *= 0.92;
        }
    }
    else
    {
        if(gyroVal < -10)
        {
            leftSpeed *= 1.15;
        }
        else if(gyroVal > 10)
        {
            rightSpeed *= 1.15;
        }
    }

    setMotors(leftMotors, leftSpeed);
    setMotors(rightMotors, rightSpeed);


}

double distReq(double speed, int numCubes, DIRECTION direction)
{
    double result = 1.0 * ROTATION_MUL;

    if (numCubes == -1)
    {
        result = 0.5 * ROTATION_MUL;
    }
    if (numCubes == -2)
    {
        result = 0.05 * ROTATION_MUL;
    }
    if (direction == BACKWARD)
    {
        result = 1.8 * ROTATION_MUL;
        if(numCubes == 8)
            result = 2.2 * ROTATION_MUL;
    }

    return result;
}

void setDirection(DIRECTION direction)
{
    if (direction == FORWARD)
    {
        wheelLeft1.set_reversed(false);
        wheelLeft2.set_reversed(true);
        wheelLeft3.set_reversed(false);
        wheelRight1.set_reversed(true);
        wheelRight2.set_reversed(false);
        wheelRight3.set_reversed(true);
    }
    if (direction == BACKWARD)
    {
        wheelLeft1.set_reversed(true);
        wheelLeft2.set_reversed(false);
        wheelLeft3.set_reversed(true);
        wheelRight1.set_reversed(false);
        wheelRight2.set_reversed(true);
        wheelRight3.set_reversed(false);
    }
}

//function used in autonomous to turn a given degree amount at a given speed
void autoTurnRelative(std::vector<pros::Motor> & leftWheelMotorVector,
    std::vector<pros::Motor> & rightWheelMotorVector, double amount)
{
  double currSpeed;
  double speed = 80;

  pros::motor_brake_mode_e_t prevBrake = leftWheelMotorVector[0].get_brake_mode();
  setBrakes(wheelMotorVector,  pros::E_MOTOR_BRAKE_BRAKE );

  amount *= 10;
  gyro.reset();
  pros::delay(100);

  float lastRemainingTicks = 99999;
  float remainingTicks = amount;
  //white both not close to target and not overshooting
  while( fabs(remainingTicks) >= 5 && fabs(remainingTicks) - fabs(lastRemainingTicks) <= 1)
  {
    double multiplier = fmax(fmin(1, fabs(remainingTicks / 1250.0)), .25);
    currSpeed = speed;
    currSpeed *= multiplier;

    if (currSpeed < 30)
        currSpeed = 30;

    if(remainingTicks < 0)
    {
      setMotors(leftWheelMotorVector, -currSpeed);
      setMotors(rightWheelMotorVector, currSpeed);
    }
    else
    {
      setMotors(leftWheelMotorVector, currSpeed);
      setMotors(rightWheelMotorVector, -currSpeed);
    }

  	if(master.get_digital(KILL_BUTTON))
    {
      break;
    }
    pros::delay(20);
    lastRemainingTicks = remainingTicks;
    remainingTicks = amount - gyro.get_value();
    pros::lcd::set_text(5, "gyro: " + std::to_string(remainingTicks));
    pros::lcd::set_text(6, "gyro: " + std::to_string(lastRemainingTicks) + " " + std::to_string(fabs(remainingTicks) <= fabs(lastRemainingTicks)));
  }

  setMotors(leftWheelMotorVector, 0);
  setMotors(rightWheelMotorVector, 0);
  pros::delay(50);

  setBrakes(wheelMotorVector,  prevBrake);

  return;
}

//turn right while adjusting speed based on distance from goal


void depositStack()
{
  //constant slow intake
  /*setMotors(intakeMotors, -20);

  //forward to align
  setMotors(leftWheelMotorVector, 30);
  setMotors(rightWheelMotorVector, 30);
  pros::delay(200);

  setMotors(leftWheelMotorVector, 0);
  setMotors(rightWheelMotorVector, 0);
  pros::delay(100);

  //slight back
  setMotors(leftWheelMotorVector, -30);
  setMotors(rightWheelMotorVector, -30);
  pros::delay(200);

  setMotors(leftWheelMotorVector, 0);
  setMotors(rightWheelMotorVector, 0);
  pros::delay(100);

  setMotors(intakeMotors, 0);*/


  setMotors(intakeMotors, -30);
  //tip up tray
  trayLeft.move_absolute(TRAY_MIDDLE_HEIGHT, 70);
  trayRight.move_absolute(TRAY_MIDDLE_HEIGHT, 70);
  while(trayLeft.get_target_position() - trayLeft.get_position() > 0 && trayRight.get_target_position() - trayRight.get_position() > 0)
  {
    if(master.get_digital(KILL_BUTTON))
    {
      break;
    }
    pros::delay(20);
  }
  pros::delay(200);

  //finish tip
  trayLeft.move_absolute(TRAY_MAX_HEIGHT, 25);
  trayRight.move_absolute(TRAY_MAX_HEIGHT, 25);
  while(trayLeft.get_target_position() - trayLeft.get_position() > 0 && trayRight.get_target_position() - trayRight.get_position() > 0)
  {
    if(master.get_digital(KILL_BUTTON))
    {
      break;
    }
    pros::delay(20);
  }

  //slight outtake
  setMotors(intakeMotors, -50);

  //smooth medium speed back
  /*setMotors(leftWheelMotorVector, -50);
  setMotors(rightWheelMotorVector, -50);
  pros::delay(400);

  setMotors(intakeMotors, 0);

  setMotors(leftWheelMotorVector, 0);
  setMotors(rightWheelMotorVector, 0);
  pros::delay(100);*/
  driveDist(0.5, BACKWARD, -1);

  setMotors(intakeMotors, 0);
  while(trayBumperLeft.get_value() == false && trayBumperRight.get_value() == false)
      setMotors(trayMotors, -75);
  setMotors(trayMotors, 0);
}

void cubeSet()
{
      setMotors(intakeMotors, 120);
      pros::delay(600);
      setMotors(intakeMotors, 0);
}

bool pressButton(std::uint32_t  & debounceTime)
{
	std::uint32_t pressTime = pros::millis();
	if(pressTime - debounceTime >= DEBOUNCE_DELAY)
	{
		debounceTime = pressTime;
		return true;
	}
	return false;
}

void unFold()
{
    //setMotors(intakeMotors, -30);
    liftLeft.move_absolute(liftPositions[0] * 0.5, 100);
    liftRight.move_absolute(liftPositions[0] * 0.5, 100);
    trayLeft.move_absolute(trayPositions[0] * 0.5, 100);
    trayRight.move_absolute(trayPositions[0] * 0.5, 100);
    pros::delay(600);
    setMotors(intakeMotors, -30);
    setMotors(liftMotors, -75);
    pros::delay(300);

    while(fabs(liftLeft.get_actual_velocity()) >= 5)
    {
        if (trayBumperLeft.get_value() == false && trayBumperRight.get_value() == false)
            setMotors(trayMotors, -40);
        pros::delay(20);
    }
    setMotors(intakeMotors, 0);
}


void gyroHold(int time)
{
    double startTime = std::clock();
    double diff = 0;
    double value = 0;
    gyro.reset();

    while (time < diff)
    {
        value = gyro.get_value();

        if (value > 5)
        {
            setMotors(leftWheelMotorVector, 30);
            setMotors(rightWheelMotorVector, -30);
        }
        if (value < -5)
        {
            setMotors(leftWheelMotorVector, -30);
            setMotors(rightWheelMotorVector, 30);
        }


        diff = std::clock() - startTime;
    }
}
