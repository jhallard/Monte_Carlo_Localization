/**
*   @File    - Controller.cpp
*   @Author  - John Allard, Alex Rich. Summer 2014.
*   @Purpose - Implementation of the controller class defined in Controller.h. This class serves as the control mechanism for the localization
*              program.
**/

#include "Controller.h"

namespace MCL
{
    Controller::Controller()
    {
        comboWeighting.push_back(1.0); //SURF
        comboWeighting.push_back(0.0); //SIFT
        comboWeighting.push_back(0.0); //GREYSCALE
        comboWeighting.push_back(0.0); // B&W
    }

    bool Controller::UpdateRobotData()
    { 
        // TODO : Grab frame from Camera callback, put the image into the robotstate class. Have the robot class process the image.
    }

    // Take the ActiveParticles class and the RobotState class, pass them to the matching function and assign weights to the particles.
    bool Controller::CompareFeatures()
    {
        vector<Particle> * temp;
        this->ap.GetParticleList(temp);
        
        for(int i = 0; i < this->ap.NumParticles(); i++)
        {   Particle * ptemp;
            ptemp = &((* temp)[i]);
            CompareAndWeigh(ptemp, this->robot, this->comboWeighting);
        }
    }
                            
    // generate a distribution based on the location and weighting of the active particles. Sample from this
    // distribution to create a new list of active particles.
    bool Controller::GenDistributionAndSample()
    {
        ap.GenerateDistribution();
        ap.GenerateParticles();
    }

    // Start by sending a movement command to the robot, then update every particle in the particle list accordingly
    bool Controller::MoveUpdate()
    {
        float xmove, ymove, zmopve;
        int thetamove = 0;

    //***** TODO - Add some sort of randomization techniques for determining the movement commands *********//
        // We need to first move the robot.
        // RobotIO::PublishMovecommand(movex, movey, thetamove)

        // We then need to move the particles.
        // this->ap.Move(movex, movey, movez, thetamove);
    }

    // Called when the program needs to wait for another part of the program to do something.
    // the argument is a pointer to a boolean flag that this function will wait to be true before moving on.
    // The second optional argument determines how long the function will wait before giving up and returning.
    bool Controller::PauseState(bool * flag, float time = 10.0)
    {
        // Wait until the flag is true to return
        for(int i = 0;!(*flag) && i < 100000; i++)
            return true;
    
        return false;
    }

    void Controller::init(string dirName)
    {
        this->ap.GetConstants(dirName);
        this->ap.SetDistribution(perspectives);
        this->ap.GenerateParticles(500);
        UpdateRobotData();
    }

    bool Controller::SpinOnce()
    {
        CompareFeatures();
        GenDistributionAndSample();
        MoveUpdate();
        UpdateRobotData();
    }

    // ActiveParticles get and set
    ActiveParticles Controller::GetActiveParticles() const      // Get the ActiveParticles class by value.
    {
        return this->ap;
    }
    void Controller::GetActiveParticles(ActiveParticles * ap2) // Get the ActiveParticles class by pointer
    {
        ap2 = &(this->ap);
    }
    bool Controller::SetActiveParticles(ActiveParticles ap2)
    {
        this->ap = ap2;
    }

    // RobotState get and set
    RobotState Controller::GetRobotState() const
    {
        return this->robot;
    }
    void Controller::GetRobotState(RobotState * newbot)
    {
        newbot = &(this->robot);
    }
    bool Controller::SetRobotState(RobotState rs)
    {
        this->robot = rs;
    }
}
