#include "SolidSphere.h"

SolidSphere sphere;

bool SolidSphere::CreateSphere()
{   
    std::stringstream ss;
    ss << "../Inputfiles/sphere/sphere.obj";
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
    scene = importer.ReadFile( pFile.c_str(), aiProcessPreset_TargetRealtime_Quality);
    if((scene == NULL))
    {
        printf("%s\n", importer.GetErrorString());
        return false;
    }
    // Now we can access the file's contents.
    printf("Import of scene %s succeeded.\n", pFile.c_str());

    // aiVector3D scene_min, scene_max, scene_center;
    // View::get_bounding_box(&scene_min, &scene_max);
    // float tmp;
    // // tmp = scene_max.x-scene_min.x;
    // // tmp = scene_max.y - scene_min.y > tmp?scene_max.y - scene_min.y:tmp;
    // // tmp = scene_max.z - scene_min.z > tmp?scene_max.z - scene_min.z:tmp;
    // // scaleFactor = 10.f / tmp;

}

    // Load textures from the texture files associated with the .obj/.3ds file. Map these textures onto the polygons.
    int SolidSphere::LoadTextures()
    {
        ILboolean success;

        /* initialization of DevIL */
        //ilInit(); 

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
            std::stringstream ss;
            ss << "../Inputfiles/sphere/"+filename;

            success = ilLoadImage((ILstring)(ss.str()).c_str());

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
                std::cout << "end Load Textures" << std::endl;
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

    void SolidSphere::recursive_render(const aiNode* nd)
    {

        // //Get node transformation matrix
        // aiMatrix4x4 m = nd->mTransformation;
        // // OpenGL matrices are column major
        // m.Transpose();

        // // save model matrix and apply node transformation
        // pushMatrix();

        // float aux[16];
        // memcpy(aux,&m,sizeof(float) * 16);
        // MathHelp::multMatrix(modelMatrix, aux);
        // setModelMatrix();

        // glTranslate(0, 0, .4);
        // draw all meshes assigned to this node
        for (unsigned int n=0; n < nd->mNumMeshes; ++n)
        {
            // bind material uniform
            glBindBufferRange(GL_UNIFORM_BUFFER, materialUniLoc, myMeshes[nd->mMeshes[n]].uniformBlockIndex, 0, sizeof(struct Helper::MyMaterial)); 
            // bind texture
            glBindTexture(GL_TEXTURE_2D, myMeshes[nd->mMeshes[n]].texIndex);
            // bind VAO
            glBindVertexArray(myMeshes[nd->mMeshes[n]].vao);
            // draw
            glDrawElements(GL_TRIANGLES,myMeshes[nd->mMeshes[n]].numFaces*3,GL_UNSIGNED_INT,0);

        }

        // draw all children
        for (unsigned int n=0; n < nd->mNumChildren; ++n)
        {
            recursive_render(nd->mChildren[n]);
        }
        // popMatrix();

    }


    void SolidSphere::genVAOsAndUniformBuffer() 
    {

        struct Helper::MyMesh aMesh;
        struct Helper::MyMaterial aMat; 
        GLuint buffer;
        
        // For each mesh
        for (unsigned int n = 0; n < scene->mNumMeshes; ++n)
        {
            const aiMesh* mesh = scene->mMeshes[n];

            // create array with faces
            // have to convert from Assimp format to array
            unsigned int *faceArray;
            faceArray = (unsigned int *)malloc(sizeof(unsigned int) * mesh->mNumFaces * 3);
            unsigned int faceIndex = 0;

            for (unsigned int t = 0; t < mesh->mNumFaces; ++t) 
            {
                const aiFace* face = &mesh->mFaces[t];

                memcpy(&faceArray[faceIndex], face->mIndices,3 * sizeof(unsigned int));
                faceIndex += 3;
            }

            aMesh.numFaces = scene->mMeshes[n]->mNumFaces;

            // generate Vertex Array for mesh
            glGenVertexArrays(1,&(aMesh.vao));
            glBindVertexArray(aMesh.vao);

            // buffer for faces
            glGenBuffers(1, &buffer);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mesh->mNumFaces * 3, faceArray, GL_STATIC_DRAW);

            // buffer for vertex positions
            if (mesh->HasPositions()) 
            {
                glGenBuffers(1, &buffer);
                glBindBuffer(GL_ARRAY_BUFFER, buffer);
                glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*mesh->mNumVertices, mesh->mVertices, GL_STATIC_DRAW);
                glEnableVertexAttribArray(vertexLoc);
                glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, 0, 0, 0);
            }

            // buffer for vertex normals
            if (mesh->HasNormals()) 
            {
                glGenBuffers(1, &buffer);
                glBindBuffer(GL_ARRAY_BUFFER, buffer);
                glBufferData(GL_ARRAY_BUFFER, sizeof(float)*3*mesh->mNumVertices, mesh->mNormals, GL_STATIC_DRAW);
                glEnableVertexAttribArray(normalLoc);
                glVertexAttribPointer(normalLoc, 3, GL_FLOAT, 0, 0, 0);
            }

            // buffer for vertex texture coordinates
            if (mesh->HasTextureCoords(0)) {
                float *texCoords = (float *)malloc(sizeof(float)*2*mesh->mNumVertices);
                for (unsigned int k = 0; k < mesh->mNumVertices; ++k) {

                    texCoords[k*2]   = mesh->mTextureCoords[0][k].x;
                    texCoords[k*2+1] = mesh->mTextureCoords[0][k].y; 
                    
                }
                glGenBuffers(1, &buffer);
                glBindBuffer(GL_ARRAY_BUFFER, buffer);
                glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*mesh->mNumVertices, texCoords, GL_STATIC_DRAW);
                glEnableVertexAttribArray(texCoordLoc);
                glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, 0, 0, 0);
            }

            // unbind buffers
            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER,0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
        
            // create material uniform buffer
            aiMaterial *mtl = scene->mMaterials[mesh->mMaterialIndex];
                
            aiString texPath;   //contains filename of texture
            if(AI_SUCCESS == mtl->GetTexture(aiTextureType_DIFFUSE, 0, &texPath))
            {
                //bind texture
                unsigned int texId = (textureIdMap)[texPath.data];
                aMesh.texIndex = texId;
                aMat.texCount = 1;
            }
            else
            {
                std::cout << "AI_SUCCESS = False for mtl->getTexture Render.cpp" << std::endl;
                aMat.texCount = 0;
            }

            float c[4];
            MathHelp::set_float4(c, 0.8f, 0.8f, 0.8f, 1.0f);
            aiColor4D diffuse;
            if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
                MathHelp::color4_to_float4(&diffuse, c);
            memcpy(aMat.diffuse, c, sizeof(c));

            MathHelp::set_float4(c, 0.2f, 0.2f, 0.2f, 1.0f);
            aiColor4D ambient;
            if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
                MathHelp::color4_to_float4(&ambient, c);
            memcpy(aMat.ambient, c, sizeof(c));

            MathHelp::set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
            aiColor4D specular;
            if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
                MathHelp::color4_to_float4(&specular, c);
            memcpy(aMat.specular, c, sizeof(c));

            MathHelp::set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
            aiColor4D emission;
            if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
                MathHelp::color4_to_float4(&emission, c);
            memcpy(aMat.emissive, c, sizeof(c));

            float shininess = 0.0;
            unsigned int max;
            aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &max);
            aMat.shininess = shininess;

            glGenBuffers(1,&(aMesh.uniformBlockIndex));
            glBindBuffer(GL_UNIFORM_BUFFER,aMesh.uniformBlockIndex);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(aMat), (void *)(&aMat), GL_STATIC_DRAW);

            myMeshes.push_back(aMesh);
        }
    }

        // gind the updated model matrix to the buffer
    void SolidSphere::setModelMatrix() 
    {
        glBindBuffer(GL_UNIFORM_BUFFER,matricesUniBuffer);
        glBufferSubData(GL_UNIFORM_BUFFER, ModelMatrixOffset, MatrixSize, modelMatrix);
        glBindBuffer(GL_UNIFORM_BUFFER,0);
    }

    // The equivalent to glTranslate applied to the model matrix
    void SolidSphere::translate(float x, float y, float z) 
    {
        float aux[16];
        MathHelp::setTranslationMatrix(aux,x,y,z);
        MathHelp::multMatrix(modelMatrix,aux);
        // Render::setModelMatrix();
    }

    // // The equivalent to glRotate applied to the model matrix
    // void SolidSphere::rotate(float angle, float x, float y, float z) 
    // {
    //     float aux[16];
    //     MathHelp::setRotationMatrix(aux,angle,x,y,z);
    //     MathHelp::multMatrix(modelMatrix,aux);
    //     Render::setModelMatrix();
    // }

    // The equivalent to glScale applied to the model matrix
    void SolidSphere::scale(float x, float y, float z)
    {
        float aux[16];
        MathHelp::setScaleMatrix(aux,x,y,z);
        MathHelp::multMatrix(modelMatrix,aux);
        // Render::setModelMatrix();
    }