/**
*   @File   - Particle.cpp
*   @Author - John Allard, Alex Rich. Summer 2014.
*   @Info   - This is the definition of the Particle class for use by the 3DLocalization program, declared in the Particle.h file. This file will define all
*           of then public and private member functions for the class. For a more in-depth description of this class, see it's section 
*           in the Documentation/CodeDocumentation directory.
**/


#include "Particle.h"

namespace MCL
{

    Particle::Particle() :
    weightmin(0), weightmax(100), defaultweight(2), perspective()
    {
        weight = defaultweight;
    }

    Particle::Particle(Perspective p) :
    weightmin(0), weightmax(100), defaultweight(2), perspective(p)
    {
        weight = defaultweight;
    }

    Particle::Particle(Perspective p, float w) :
    weightmin(0), weightmax(100), defaultweight(2), perspective(p)
    {
        if(w >= weightmin && w <= weightmax)
            weight = w;
        else
            weight = defaultweight;
    }

    //====Destructor====//
    Particle::~Particle()
    {
     int x = 0;
    }

    std::string Particle::ToString()
    {
        std::stringstream ss;
        ss << this->perspective.ToString() << " Weight: " << weight;
        return ss.str();
    }

    //====Get and Set====//
    bool Particle::SetPerspective(Perspective p)
    {
        this->perspective = p;
    }

    MCL::Perspective Particle::GetPerspective() const
    {
        return this->perspective;
    }

    float Particle::GetPerspective(int index) const
    {
        if(index >= 0 && index < 6)
        {
            std::vector<float> temp = perspective.ToVector();
            return temp[index];
        }
        else
        {
            std::string temp = "Invalid Index Argument in GetPerspective Function Call";
            MCL::ErrorIO(temp);
            throw std::logic_error(temp);
        }
    }

    float Particle::GetWeight() const
    {
        return this->weight;
    }

    bool Particle::SetWeight(float w)
    {
        weight = w; 
        return true;
        if(w >= weightmin && w <= weightmax)
        {
            weight = w;
            return true;
        }
        else
            return false;
    }

    float Particle::Distance(Particle p)
    {
        float dx = p.GetPerspective(0) - this->GetPerspective(0);
        float dy = p.GetPerspective(1) - this->GetPerspective(1);
        float dz = p.GetPerspective(2) - this->GetPerspective(2);
        
        return sqrt(dx * dx + dy * dy + dz * dz);
    }


}
