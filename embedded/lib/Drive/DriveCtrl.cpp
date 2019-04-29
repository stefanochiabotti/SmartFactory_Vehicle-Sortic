/**
 * @file DriveCtrl.cpp
 * @brief Implementation of the Drive Controll-Class
 * 
 * @author Luca Mazzoleni (luca.mazzoleni@hsr.ch)
 * 
 * @version 1.0 -Implementation Drive Controll FSM - Luca Mazzoleni (luca.mazzoleni@hsr.ch) - 2019-04-23
 * 
 * @date 2019-04-23
 * @copyright Copyright (c) 2019
 * 
 * @todo solve linedetection with something like first position and not with loopcount
 */

#include "DriveCtrl.h"
//=====PUBLIC====================================================================================
DriveCtrl::DriveCtrl() : currentState(State::idle) {
    pController.SetTunings(pVal_p, pVal_i, pVal_d);
    pController.SetOutputLimits(-255, 255);
    pController.SetSampleTime(pSampleTime);  // Sampletime in milliseconds
    pController.SetMode(AUTOMATIC);
    pController.SetControllerDirection(DIRECT);
    DBINFO2ln(String("Kp: ") + String(pController.GetKp()) + String(" Ki: ") + String(pController.GetKi()) + String(" Kd: ") + String(pController.GetKd()));
}

void DriveCtrl::loop() {
    DBFUNCCALLln("DriveCtrl::loop()");
    process((this->*doActionFPtr)());  //do actions
}

void DriveCtrl::loop(Event currentEvent) {
    DBFUNCCALLln("DriveCtrl::loop(Event)");
    process(currentEvent);
    process((this->*doActionFPtr)());  //do actions
}

const DriveCtrl::State DriveCtrl::getcurrentState() {
    return currentState;
}
//=====PRIVATE====================================================================================
void DriveCtrl::process(Event e) {
    DBFUNCCALL("DriveCtrl::process ");
    DBEVENTln(String("DriveCtrl ") + String(decodeEvent(e)));
    switch (currentState) {
        case State::idle:
            if (Event::TurnLeft == e) {
                exitAction_idle();          // Exit-action current state
                entryAction_turningLeft();  // Entry-actions next state
            } else if (Event::TurnLeft == e) {
                exitAction_idle();          // Exit-action current state
                entryAction_turningLeft();  // Entry-actions next state
            } else if (Event::TurnRight == e) {
                exitAction_idle();           // Exit-action current state
                entryAction_turningRight();  // Entry-actions next state
            } else if (Event::TurnAround == e) {
                exitAction_idle();            // Exit-action current state
                entryAction_turningAround();  // Entry-actions next state
            } else if (Event::FollowLine == e) {
                exitAction_idle();            // Exit-action current state
                entryAction_followingLine();  // Entry-actions next state
            } else if (Event::Error == e) {
                exitAction_idle();         // Exit-action current state
                entryAction_errorState();  // Entry-actions next state
            }
            break;
        case State::turningLeft:
            if (Event::LineAligned == e) {
                exitAction_turningLeft();  // Exit-action current state
                entryAction_idle();        // Entry-actions next state
            } else if (Event::Error == e) {
                exitAction_turningLeft();  // Exit-action current state
                entryAction_errorState();  // Entry-actions next state
            }
            break;
        case State::turningRight:
            if (Event::LineAligned == e) {
                exitAction_turningRight();  // Exit-action current state
                entryAction_idle();         // Entry-actions next state
            } else if (Event::Error == e) {
                exitAction_turningRight();  // Exit-action current state
                entryAction_errorState();   // Entry-actions next state
            }
            break;
        case State::turningAround:
            if (Event::LineAligned == e) {
                exitAction_turningAround();  // Exit-action current state
                entryAction_idle();          // Entry-actions next state
            } else if (Event::Error == e) {
                exitAction_turningAround();  // Exit-action current state
                entryAction_errorState();    // Entry-actions next state
            }
            break;
        case State::followingLine:
            if (Event::FullLineDetected == e) {
                exitAction_followingLine();  // Exit-action current state
                entryAction_idle();          // Entry-actions next state
            } else if (Event::Error == e) {
                exitAction_followingLine();  // Exit-action current state
                entryAction_errorState();    // Entry-actions next state
            }
            break;
        case State::errorState:
            if (Event::Resume == e) {
                exitAction_errorState();  // Exit-action current state
                switch (lastStateBevorError) {
                    case State::idle:
                        entryAction_idle();  // Entry-actions next state
                        break;
                    case State::turningLeft:
                        entryAction_turningLeft();  // Entry-actions next state
                        break;
                    case State::turningRight:
                        entryAction_turningRight();  // Entry-actions next state
                        break;
                    case State::turningAround:
                        entryAction_turningAround();  // Entry-actions next state
                        break;
                    case State::followingLine:
                        entryAction_followingLine();  // Entry-actions next state
                        break;
                    default:
                        break;
                }
            }
        default:
            break;
    }
}
//==idle==========================================================
void DriveCtrl::entryAction_idle() {
    DBSTATUSln("Drive Entering State: idle");
    currentState = State::idle;  // state transition
    doActionFPtr = &DriveCtrl::doAction_idle;
    //Entry-Action
}

DriveCtrl::Event DriveCtrl::doAction_idle() {
    DBINFO1ln("Drive State: idle");
    //Generate the Event
    return Event::NoEvent;
}

void DriveCtrl::exitAction_idle() {
    DBSTATUSln("Drive Leaving State:  idle");
}

//==turningLeft==========================================================
void DriveCtrl::entryAction_turningLeft() {
    DBSTATUSln("Drive Entering State: turningLeft");
    currentState = State::turningLeft;  // state transition
    doActionFPtr = &DriveCtrl::doAction_turningLeft;
    loopcount = 0;
    //Entry-Action
    pDrive.turn(Drive::Direction::Left, TURNING_SPEED);
}

DriveCtrl::Event DriveCtrl::doAction_turningLeft() {
    DBINFO1ln("Drive State: turningLeft");
    //Generate the Event

    if ((abs(pEnvdetect.Linedeviation()) < 1) && (loopcount > 5)) {
        return Event::LineAligned;
    }
    loopcount += 1;
    return Event::NoEvent;
}

void DriveCtrl::exitAction_turningLeft() {
    DBSTATUSln("Drive Leaving State:  turningLeft");
    pDrive.stop();
}

//==turningRight==========================================================
void DriveCtrl::entryAction_turningRight() {
    DBSTATUSln("Drive Entering State: turningRight");
    currentState = State::turningRight;  // state transition
    doActionFPtr = &DriveCtrl::doAction_turningRight;
    loopcount = 0;
    //Entry-Action
    pDrive.turn(Drive::Direction::Right, TURNING_SPEED);
}

DriveCtrl::Event DriveCtrl::doAction_turningRight() {
    DBINFO1ln("Drive State: turningRight");
    //Generate Event
    if ((abs(pEnvdetect.Linedeviation()) < 1) && (loopcount > 5)) {
        return Event::LineAligned;
    }
    loopcount += 1;
    return Event::NoEvent;
}

void DriveCtrl::exitAction_turningRight() {
    DBSTATUSln("Drive Leaving State:  turningRight");
    pDrive.stop();
}

//==turningAround==========================================================
void DriveCtrl::entryAction_turningAround() {
    DBSTATUSln("Drive Entering State: turningAround");
    currentState = State::turningAround;  // state transition
    doActionFPtr = &DriveCtrl::doAction_turningAround;
    loopcount = 0;
    //Entry-Action
    pDrive.turnonpoint(Drive::Direction::Right, TURNING_SPEED);
}

DriveCtrl::Event DriveCtrl::doAction_turningAround() {
    DBINFO1ln("Drive State: turningAround");
    //Generate the Event
    DBINFO2ln(loopcount);
    if ((abs(pEnvdetect.Linedeviation()) < 1) && (loopcount > 5)) {
        return Event::LineAligned;
    }
    loopcount += 1;
    return Event::NoEvent;
}

void DriveCtrl::exitAction_turningAround() {
    DBSTATUSln("Drive Leaving State:  turningAround");
    pDrive.stop();
}

//==followingLine==========================================================
void DriveCtrl::entryAction_followingLine() {
    DBSTATUSln("Drive Entering State: followingLine");
    currentState = State::followingLine;  // state transition
    doActionFPtr = &DriveCtrl::doAction_followingLine;
    loopcount = 0;
    pController_Input = 0;
    //Entry-Action
    pDrive.drive(Drive::Direction::Forward, SPEED);
}

DriveCtrl::Event DriveCtrl::doAction_followingLine() {
    DBINFO1ln("Drive State: followingLine");
    //Generate the Event
    // DBINFO3ln(pEnvdetect.Linedeviation());
    int linedeviation = pEnvdetect.Linedeviation();
    if ((abs(linedeviation) == 180) && (loopcount > 5)) {
        return Event::FullLineDetected;
    } else if ((abs(linedeviation) == 200)) {
        //Line lost
        // return Event::FullLineDetected;
    }
    if ((linedeviation != 180) && (linedeviation != 200)) {
        pController_Input = linedeviation;
    }
    DBINFO2ln(String("Controll-Input: ") + String(pController_Input));
    pController.Compute();
    DBINFO2ln(String("Controll-Correction: ") + String(pController_Output));
    if (pController_Output > 0) {
        pDrive.turn(Drive::Direction::Left, abs(pController_Output));
    } else if (pController_Output < 0) {
        pDrive.turn(Drive::Direction::Right, abs(pController_Output));
    }

    loopcount += 1;
    return Event::NoEvent;
}

void DriveCtrl::exitAction_followingLine() {
    DBSTATUSln("Drive Leaving State:  followingLine");
    pDrive.stop();
}

//==errorState========================================================
void DriveCtrl::entryAction_errorState() {
    DBERROR("Drive Entering State: errorState");
    lastStateBevorError = currentState;
    currentState = State::errorState;  // state transition
    doActionFPtr = &DriveCtrl::doAction_errorState;
    //Entry-Action
    pDrive.stop();
}

DriveCtrl::Event DriveCtrl::doAction_errorState() {
    DBSTATUSln("Drive State: errorState");
    //Generate the Event
    return Event::NoEvent;
}

void DriveCtrl::exitAction_errorState() {
    DBSTATUSln("Drive Leaving State:  errorState");
}

//============================================================================
//==Aux-Function==============================================================
String DriveCtrl::decodeState(State state) {
    switch (state) {
        case State::idle:
            return "State::idle";
            break;
        case State::turningLeft:
            return "State::turningLeft";
        case State::turningRight:
            return "State::turningRight";
            break;
        case State::turningAround:
            return "State::turningAround";
            break;
        case State::errorState:
            return "State::errorState";
            break;
        default:
            return "ERROR: No matching state";
            break;
    }
}

String DriveCtrl::decodeEvent(Event event) {
    switch (event) {
        case Event::TurnLeft:
            return "Event::TurnLeft";
            break;
        case Event::TurnRight:
            return "Event::TurnRight";
            break;
        case Event::TurnAround:
            return "Event::TurnAround";
            break;
        case Event::FollowLine:
            return "Event::FollowLine";
            break;
        case Event::LineAligned:
            return "Event::LineAligned";
            break;
        case Event::FullLineDetected:
            return "Event::FullLineDetected";
            break;
        case Event::Error:
            return "Event::Error";
            break;
        case Event::Resume:
            return "Event::Resume";
            break;
        case Event::NoEvent:
            return "Event::NoEvent";
            break;
        default:
            return "ERROR: No matching event";
            break;
    }
}