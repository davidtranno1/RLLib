/*
 * Copyright 2013 Saminda Abeyruwan (saminda@cs.miami.edu)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SwingPendulum.h
 *
 *  Created on: Aug 25, 2012
 *      Author: sam
 */

#ifndef SWINGPENDULUM_H_
#define SWINGPENDULUM_H_

#include "RL.h"

using namespace RLLib;

template<class T>
class SwingPendulum: public RLProblem<T>
{
    typedef RLProblem<T> Base;
  protected:
    float uMax, stepTime, theta, velocity, maxVelocity;

    Range<float>* actionRange;
    Range<float>* thetaRange;
    Range<float>* velocityRange;

    float mass, length, g, requiredUpTime, upRange;

    int upTime;
    bool random;
  public:
    SwingPendulum(const bool& random = false) :
        RLProblem<T>(2, 3, 1), uMax(2.0/*Doya's paper 5.0*/), stepTime(0.01), theta(0), velocity(0), maxVelocity(
        M_PI_4 / stepTime), actionRange(new Range<float>(-uMax, uMax)), thetaRange(
            new Range<float>(-M_PI, M_PI)), velocityRange(
            new Range<float>(-maxVelocity, maxVelocity)), mass(1.0), length(1.0), g(9.8), requiredUpTime(
            10.0 /*seconds*/), upRange(M_PI_4 /*seconds*/), upTime(0), random(random)
    {

      Base::discreteActions->push_back(0, actionRange->min());
      Base::discreteActions->push_back(1, 0.0);
      Base::discreteActions->push_back(2, actionRange->max());

      // subject to change
      Base::continuousActions->push_back(0, 0.0);

      for (int i = 0; i < this->dimension(); i++)
        Base::resolutions->at(i) = 10.0;

    }

    virtual ~SwingPendulum()
    {
      delete actionRange;
      delete thetaRange;
      delete velocityRange;
    }

  private:
    void adjustTheta()
    {
      if (theta >= M_PI)
        theta -= 2.0 * M_PI;
      if (theta < -M_PI)
        theta += 2.0 * M_PI;
    }

  public:
    void updateRTStep()
    {
      DenseVector<T>& vars = *Base::output->o_tp1;
      //std::cout << (theta * 180 / M_PI) << " " << xDot << std::endl;
      vars[0] = (theta - thetaRange->min()) * Base::resolutions->at(0) / thetaRange->length();
      vars[1] = (velocity - velocityRange->min()) * Base::resolutions->at(1)
          / velocityRange->length();

      Base::observations->at(0) = theta;
      Base::observations->at(1) = velocity;

      Base::output->updateRTStep(r(), z(), endOfEpisode());
    }
    void initialize()
    {
      upTime = 0;
      if (random)
        theta = thetaRange->chooseRandom();
      else
        theta = M_PI_2;
      velocity = 0.0;
      adjustTheta();
      updateRTStep();
    }

    void step(const Action<double>* a)
    {
      //std::cout << a.at() << std::endl;
      float torque = actionRange->bound(a->at(0));
      float thetaAcc = -stepTime * velocity + mass * g * length * sin(theta) + torque;
      velocity = velocityRange->bound(velocity + thetaAcc);
      theta += velocity * stepTime;
      adjustTheta();
      upTime = fabs(theta) > upRange ? 0 : upTime + 1;

      updateRTStep();
    }
    bool endOfEpisode() const
    {
      return false;
      //return upTime + 1 >= requiredUpTime / stepTime; // 1000 steps
    }
    float r() const
    {
      return cos(theta);
    }
    float z() const
    {
      return 0.0;
    }

};

#endif /* SWINGPENDULUM_H_ */
