#include "ProgramIO.h"
#include "boost/filesystem.hpp"


Assimp::Importer importer;
float r = 5.0;
int startX = 0;
int startY = 0;
int  tracking = 0;
float alpha = 0.0f;
float beta = 0.0f;

namespace IO
{
     float round(float x)
    {
        return (float) ((int) (x*100))/100.0;
    }

    // @Input - path to the input file relative to the build folder of this project
    // @Output - bool letting the program know if everything succeeded okay
    bool parseInputFile(const char * name)
    {
        double values[6] = {0, 0, 0, 0, 0, 0}; // contains the x, y, z and perspective coordinated
        
        std::ifstream file(name);

        if (!file.is_open())
        {   std::cout << "ParseInput file not found\n" << std::endl;
            return false;
        }
        // get the camxyz and lookat values.
        while(!file.eof())
        {
            std::string str;
            int j = 0;
            double value = 0;                         // the value of the item at the below index
            int count = 0;                            // the current index in the below array

            getline(file, str);
            for(int i = 0; i < str.size() && count < 6; i++)
            {
                if(str[i] == ' ' || str[i] == ',')
                {   
                    std::string temp;
                    temp = str.substr(j, i);
                    value = atof(temp.c_str());
                    values[count] = value;
                    count++;
                    j = i;
                }
            }
            std::vector<float> temp;
            for(int i = 0; i < 6; i++)
                temp.push_back(values[i]);

            perspectiveList.push_back(temp);
            perspectiveCount++;
        }

        std::cout << perspectiveCount << " Perspectives Detected" << std::endl;

        camera[0] = perspectiveList[0][0];
        camera[1] = perspectiveList[0][1];
        camera[2] = perspectiveList[0][2];
        translation[0] = values[3];
        translation[1] = values[4];
        translation[2] = values[5];

        return true;

    }


    bool generatePoints(char * name)
    {
        double minx = 0;
        double maxx = 0;
        double miny = 0;
        double maxy = 0;
        double z = 0;
        double dif = 0;
        double theta = 0;

        double values[7] = {0, 0, 0, 0, 0, 0, 0};

        std::ifstream file(name);

        if (!file.is_open())
            return false;

        std::string str;
        // get the name of the .obj folder
        getline( file, str);
        modelDirectoryName = str;


        // get the name of the obj file
        getline( file, str);
        modelname = str;

        boost::filesystem::path old_path(boost::filesystem::current_path());
        boost::filesystem::current_path(generatedImagesDirectory);
        boost::filesystem::path ndir(modelDirectoryName);

        if(!boost::filesystem::exists(ndir))
            if(boost::filesystem::create_directory(ndir));
                std::cout << "New Rendered Images Directory Created : "  << ndir << std::endl;

        boost::filesystem::current_path(old_path);

        boost::filesystem::current_path(generatedInputDirectory);
        boost::filesystem::path ndir2(modelDirectoryName);

        if(!boost::filesystem::exists(ndir2))
            if(boost::filesystem::create_directory(ndir2));
                std::cout << "New Generated Input Directory Created :" << ndir2 << std::endl;

        boost::filesystem::current_path(old_path);

        int count = 0;
        int j = 0;
        // get the bounding box values
        getline(file, str);
        for(int i = 0; i < str.size() && count < 7; i++)
        {
            if(str[i] == ' ' || str[i] == ',')
            {   
                std::string temp;
                temp = str.substr(j, i);
                double value = atof(temp.c_str());
                values[count] = value;
                count++;
                j = i;
            }
        }
        file.close();

        minx = values[0]; maxx = values[1];
        miny = values[2]; maxy = values[3];
        z = values[4]; dif = values[5];
        theta = values[6];
        int ntheta = (int) theta;

        if(ntheta < 0)
            ntheta *= -1;
        if(ntheta >= 360)
            ntheta = ntheta%360;

        if(ntheta <= 12)
            ntheta = 10;
        else if(ntheta <= 24)
            ntheta = 15;
        else if(ntheta <= 34)
            ntheta = 30;
        else if(ntheta <= 53)
            ntheta = 45;
        else if(ntheta <= 75)
            ntheta = 60;
        else
            ntheta = 90;

        std::ofstream generatedInputfile;
        std::stringstream sss;
        sss << generatedInputDirectory << modelDirectoryName << "InputInfo.txt";
        generatedInputfile.open(sss.str().c_str());

        generatedInputfile << dif << "\n";
        generatedInputfile << theta << "\n";

        generatedInputfile.close();



        std::cout << "Input File Successfully Parsed. Generating Perspectives. " <<  std::endl;

        std::string filename;
        std::stringstream ss;
        ss << "../Inputfiles/CurrentPoints.txt"; // internally generated file to be used by other parts of this program
        filename = ss.str();ss.str("");

        std::ofstream file1;
        file1.open(filename.c_str());

        // Go through the bounding box given by the user and generated distinct points at each grid intersection.
        // Skip the ones predefined to be inside walls or other undesirable areas.
        double x = minx;
        double y = miny;
        for(int i = 0; i <= (maxx-minx)/dif; i++)
        {
            for(int j = 0; j <= (maxy-miny)/dif; j++)
            {
                // if((x >= -1.9 && x <= 0) && (y >= 2.2 && y <= 5.75))
                //     continue;
                // else if((x >= -5 && x <= -3) && (y >= -0.4 && y <= 1.65))
                //     continue;
                // else if((x >= -5 && x <= -3.2) && (y >= 6.7 && y <= 9))
                //     continue;

                int currentangle = 0;
                for(int i = 0; i < (360/ntheta); i++)
                {
                    double tempangle = (double)currentangle*3.14159/180.0;
                    ss << x << " " << y << " " << z << " " <<  round(cos(tempangle)) << " " << round(sin(tempangle)) << " " << 0 << " " << "\n";
                    file1 << ss.str();ss.str("");
                    currentangle+=ntheta;
                }
                y = miny+dif*j;
            }
            x = minx+i*dif;
        }
        file1.close();

        if(!parseInputFile(filename.c_str()))
        {
            std::cout << "Error Generating Output Points for Rendering. Please check directory names, or report a bug \n" <<
            "at https://github.com/jhallard/3DLocalization" << std::endl;
            return false;
        }

        return true;
    }

    // Finds and loads the .obj file that contains the 3D model data.
    // Pass it the full file name of the .obj file relative to the build folder!
    bool Import3DFromFile()//, const aiScene* scene)
    {
        std::stringstream ss;
        ss << pathToModelDataDirectory << modelDirectoryName << modelname;
        std::cout << ss.str() << std::endl;
        std::string pFile = ss.str();ss.str("");

        std::ifstream fin(pFile.c_str());
        if(!fin.fail()) {
            fin.close();
            std::cout << ".obj File Located\n";
        }
        else
        {
            printf("Couldn't open file: %s\n", pFile.c_str());
            printf("%s\n", importer.GetErrorString());
            return false;
        }
        std::cout << pFile.c_str() << std::endl;
        scene = importer.ReadFile( pFile.c_str(), aiProcessPreset_TargetRealtime_Quality);
        if((scene == NULL))
        {
            printf("%s\n", importer.GetErrorString());
            return false;
        }
        // Now we can access the file's contents.
        printf("Import of scene %s succeeded.\n", pFile.c_str());

        aiVector3D scene_min, scene_max, scene_center;
        View::get_bounding_box(&scene_min, &scene_max);
        float tmp;
        tmp = scene_max.x-scene_min.x;
        tmp = scene_max.y - scene_min.y > tmp?scene_max.y - scene_min.y:tmp;
        tmp = scene_max.z - scene_min.z > tmp?scene_max.z - scene_min.z:tmp;
        scaleFactor = 10.f / tmp;

        return true;
    }


    // Load textures from the texture files associated with the .obj/.3ds file. Map these textures onto the polygons.
    int LoadGLTextures()
    {
        ILboolean success;

        /* initialization of DevIL */
        ilInit(); 

        if(scene == NULL)
        {
            std::cout << "scene is null inside load textures\n" << std::endl;
            return -1;
        }
        /* scan scene's materials for textures */
        for (unsigned int m=0; m<scene->mNumMaterials; ++m)
        {
            int texIndex = 0;
            aiString path;  // filename

            aiReturn texFound = scene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
            while (texFound == AI_SUCCESS)
            {
                //fill map with textures, OpenGL image ids set to 0
                textureIdMap[path.data] = 0; 
                // more textures?
                texIndex++;
                texFound = scene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
            }
        }


        int numTextures = textureIdMap.size();

        if(numTextures == 0)
            std::cout << "0 textures inside programIO.cpp" << std::endl;

        /* create and fill array with DevIL texture ids */
        ILuint* imageIds = new ILuint[numTextures];
        ilGenImages(numTextures, imageIds); 

        /* create and fill array with GL texture ids */
        GLuint* textureIds = new GLuint[numTextures];
        glGenTextures(numTextures, textureIds); /* Texture name generation */

        /* get iterator */
        std::map<std::string, GLuint>::iterator itr = textureIdMap.begin();

        for (int i=0; itr != textureIdMap.end(); ++i, ++itr)
        {
            //save IL image ID
            std::string filename = (*itr).first;  // get filename
            (*itr).second = textureIds[i];    // save texture id for filename in map

            ilBindImage(imageIds[i]); /* Binding of DevIL image name */
            ilEnable(IL_ORIGIN_SET);
            ilOriginFunc(IL_ORIGIN_LOWER_LEFT); 
            success = ilLoadImage((ILstring)(pathToModelDataDirectory+modelDirectoryName+filename).c_str());

            if (success)
            {
                /* Convert image to RGBA */
                ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE); 

                /* Create and load textures to OpenGL */
                glBindTexture(GL_TEXTURE_2D, textureIds[i]); 
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ilGetInteger(IL_IMAGE_WIDTH),
                    ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGBA, GL_UNSIGNED_BYTE,
                    ilGetData()); 
            }
            else 
                printf("Couldn't load Image: %s\n", filename.c_str());
        }
        /* Because we have already copied image data into texture data
        we can release memory used by image. */
        ilDeleteImages(numTextures, imageIds); 

        //Cleanup
        delete [] imageIds;
        delete [] textureIds;

        //return success;
        return true;
    }



    void printProgramInfoLog(GLuint obj)
    {
        int infologLength = 0;
        int charsWritten  = 0;
        char *infoLog;

        glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

        if (infologLength > 0)
        {
            infoLog = (char *)malloc(infologLength);
            glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
            printf("%s\n",infoLog);
            free(infoLog);
        }
    }


    void changeSize(int w, int h)
     {

        float ratio;
        // Prevent a divide by zero, when window is too short
        // (you cant make a window of zero width).
        if(h == 0)
            h = 1;

        // Set the viewport to be the entire window
        glViewport(0, 0, w, h);

        ratio = (1.0f * w) / h;
        View::buildProjectionMatrix(60.13f, ratio, 0.0001f, 100.0f);
    }

    #define printOpenGLError() printOglError(__FILE__, __LINE__)
    int printOglError(char *file, int line)
    {
        
        GLenum glErr;
        int retCode = 0;

        glErr = glGetError();
        if (glErr != GL_NO_ERROR)
        {
            printf("glError in file %s @ line %d: %s\n", file, line, gluErrorString(glErr));
            retCode = 1;
        }
        return retCode;
    }


    void nextLocation(int)
    {
         currentPerspective++;
        if(currentPerspective >= perspectiveCount)
            currentPerspective = 0;

        //std::cout << "Switch to perspective #" << currentPerspective << std::endl;

        std::vector<float> temp = perspectiveList[currentPerspective];

        if(temp.size() == 6)
        {
            camera[0] = temp[0];
            camera[1] = temp[1];
            camera[2] = temp[2];
            translation[0] = temp[3];
            translation[1] = temp[4];
            translation[2] = temp[5];
        }
        else
        {
            std::cout << "need 6 values, didn't get 6" << std::endl;
        }

        std::cout << "#" << currentPerspective << " [" << camera[0] << " " << camera[1] << " " << camera[2] << "]" << std::endl;
        //glutPostRedisplay();
    }


    void processKeys(unsigned char key, int xx, int yy) 
    {
        double f = .05;
        switch(key)
        {
            case 27: glutLeaveMainLoop();break;
            case ' ': start = false;break;//nextLocation(); break;
            case 'n': nextLocation(1);break;
        }
        // camera[0] = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
        // camera[1] = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
        // camera[2] = r *                               sin(beta * 3.14f / 180.0f);
    }


    void processMouseButtons(int button, int state, int xx, int yy) 
    {
        //glutTimerFunc(50, nextLocation, 1);
        start = true;
    }

    // Track mouse motion while buttons are pressed

    void processMouseMotion(int xx, int yy)
    {

        int deltaX, deltaY;
        float alphaAux, betaAux;
        float rAux;

        deltaX =  startX - xx;
        deltaY =  yy - startY;
        // left mouse button: move camera
        if (tracking == 1) 
        {
            // alphaAux = alpha + deltaX;
            // betaAux = beta + deltaY;

            // if (betaAux > 85.0f)
            //     betaAux = 85.0f;
            // else if (betaAux < -85.0f)
            //     betaAux = -85.0f;

            // rAux = r;

            // camera[0] = rAux * cos(betaAux * 3.14f / 180.0f) * sin(alphaAux * 3.14f / 180.0f);
            // camera[1] = rAux * cos(betaAux * 3.14f / 180.0f) * cos(alphaAux * 3.14f / 180.0f);
            // camera[2] = rAux * sin(betaAux * 3.14f / 180.0f);
        }
        else if (tracking == 2)
         {
            // alphaAux = alpha;
            // betaAux = beta;
            // rAux = r + (deltaY * 0.01f);

            // camera[0] = rAux * cos(betaAux * 3.14f / 180.0f) * sin(alphaAux * 3.14f / 180.0f);
            // camera[1] = rAux * cos(betaAux * 3.14f / 180.0f) * cos(alphaAux * 3.14f / 180.0f);
            // camera[2] = rAux * sin(betaAux * 3.14f / 180.0f);
         }
    }




    void mouseWheel(int wheel, int direction, int x, int y) 
    {

        r += direction * 0.1f;
        camera[0] = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
        camera[1] = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
        camera[2] =  r *                             sin(beta * 3.14f / 180.0f);
    }



} // end namespace IO
