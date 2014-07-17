/**
*   @File    - Controller.cpp
*   @Author  - John Allard, Alex Rich. Summer 2014.
*   @Purpose - Implementation of the controller class defined in Controller.h. This class serves as the control mechanism for the localization
*              program.
**/

#include "Controller.h"

namespace MCL
{
    //static variable definitions
    Mat Controller::nextImage;

    Controller::Controller() :
    publish("MCL_Publisher"), // name of the MCL-related data publisher that we must publish under.
    subscriber("ROBOT_DATA") // Name of the robot movement data publisher we must subscribe to and the user must publish under

    {
        ros::init(argc, argv, "3DLocalization"); // Init must be called before making a node handle
        rosNodeHandle = new ros::NodeHandle();   // now throw the node handle on the stack
        comboWeighting.push_back(1.0); //SURF
        comboWeighting.push_back(0.0); //SIFT
        comboWeighting.push_back(0.0); //GREYSCALE
        comboWeighting.push_back(0.0); // B&W
    }

    Controller::~Controller()
    {
        delete rosNodeHandle;
    }

    bool Controller::UpdateRobotData()
    { 
        // TODO : Grab frame from Camera callback, put the image into the robotstate class. Have the robot class process the image.
        // Mat im = imread("../../../image.jpg");
        if(!this->nextImage.data)
        {
            ErrorIO("NOIMAGE");
            return false;
        }
        else
            robot.GenerateCharacterizer(im);
        return true;
    }

    // Take the ActiveParticles class and the RobotState class, pass them to the matching function and assign weights to the particles.
    bool Controller::CompareFeatures()
    {
        vector<Particle> v = this->ap.GetParticleList();

        for(int i = 0; i < this->ap.NumParticles(); i++)
        {   
            float wt = CompareAndWeigh(v[i], this->robot, this->comboWeighting);
            v[i].SetWeight(wt);
        }

        this->ap.SetParticleList(v);
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

    bool Controller::init(string dirName, int argc, char ** argv)
    {
        RobotInit(argc, argv);

        if(!this->ap.GetConstants(dirName))
            return false;
        this->ap.SetDistribution(perspectives);
        this->ap.GenerateParticles(10);
        this->ap.AnalyzeList();
        UpdateRobotData();

        return true;
    }

    bool Controller::SpinOnce()
    {
        CompareFeatures();
        GenDistributionAndSample();
        MoveUpdate();
        UpdateRobotData();
        this->ap.AnalyzeList();
    }


    //==========================================================================//
    // ====== ROBOT COMMUNICATION CALLBACKS AND INITIALIZATION FUNCTIONS ====== //
    //==========================================================================//

    // recieves and processes images that are published from the robot or other actor. It is up to the user to set up
    // their own publisher and to convert their image data to a sensor_msgs::Image::ConstPtr& .
    void Controller::ImageCallback(const sensor_msgs::Image::ConstPtr& img)
    {
        cv_bridge::CvImagePtr im2;
        try
        {
            im2 = cv_bridge::toCvCopy(img, sensor_msgs::image_encodings::BGR8);
        }
        catch (...)
        {
            ErrorIO("ImageCallback toCvCopy call failed");
        }

        this->nextImage = im2->image;
        //SetNextInputImage(im2->image);
    }

    // Called when the robot program is 
    bool Controller::RobotInit(int argc, char ** argv)
    {
        mclDataPublisher = this->rosNodePtr->advertise<std_msgs::String>(MCL_PUBLISHER_NAME, 4);

        bool connection_succeded = this->PauseState(&(mclDataPublisher->getNumSubscribers()), 10);

        if(!connection_succeded)
        {
            ErrorIO("Robot Program Failed to Subscribe to mclDataPublisher after 10 seconds. (RobotInit(..))");
            return false;
        }
        else
            UserIO("Robot Has Subscribed to mclDataPublisher.")

        // ++++ TODO - Send a Handshake greeting to the robot program - TODO ++++ //
        std_msgs::String msg;
        std::stringstream ss;
        ss << "Initializing" << std::endl;
        msg.data = ss.str();
        mcl_data_publisher.publish(msg);
        // ++++ TODO - Send a Handshake greeting to the robot program - TODO ++++ //


        robotMovementSubscriber = this->rosNodePtr->subscribe(this->ROBOT_PUBLISHER_NAME, 2, &Controller::ImageCallback, this);

        // wait max 5 seconds for the subscriber to connect.
        connection_succeded = this->PauseState(&(robotMovementSubscriber->getNumPublishers()), 5);

        if(!connection_succeded)
        {
            ErrorIO("Robot Movement Publisher Failed to be Detected after 10 seconds of waiting. (RobotInit(..))");
            return false;
        }
        else
            UserIO("Localization Program Has Subscribed to the Robot Data Publisher.")

        return ros::ok();
    }

    bool Controller::PublishData(int code, std::string str)
    {
        std_msgs::String msg;
        msg.data = str;
        data_publisher.publish(msg);
        MCL::DebugIO("Data Published to Robot");
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
