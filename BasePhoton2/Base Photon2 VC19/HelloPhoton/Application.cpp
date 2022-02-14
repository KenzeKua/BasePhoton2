#include "Application.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include "MyPhoton.h"

using namespace std;

// The higher networkFPS, the higher bandwidth requirement of your game.
// ** about 30FPS is common data sync rate for real time games
// ** slower-paced action games could use as low as 10 FPS

const float networkFPS = 20.0f;
const float gNetworkFrameTime = 1.0f/networkFPS;

Application::Application ()
{
	/*
	method 0 : without any interpolation or prediction (send/receive position only)
	method 1 : simple interpolation (send/receive position only)
	method 2 : prediction + interpolation 1 (send/receive position only, data saving hybrid method, 
											 the result is like in between method1 and method3)
	method 3 : prediction + interpolation 2 (send/receive position, velocity and acceleration information for more precise prediction, 
  											 3 times higher data bandwidth requirement compare with method 0-2, more responsive than method 2)
	*/

	m_method = 0;
}

Application::~Application ()
{	
}

void Application::start ()
{
	srand(time(0));

	m_gameStarted = false;
	
	MyPhoton::getInstance().connect();

	m_sprite01 = new Sprite ("../media/Particle 01.bmp");	

	m_object1.setSprite(m_sprite01);
	m_object1.setPos(Vector(rand()%800,rand()%600,0));
	m_object1.setBlendMode(ADDITIVE);

	m_object2.setSprite(m_sprite01);
	m_object2.setPos(Vector(200, 200, 0));
	m_object2.setBlendMode(ADDITIVE);

	m_lastReceivedPos = m_object2.getPos();
}

void Application::sendMyData(void)
{
	Vector pos =  m_object1.getPos();
	Vector vel = m_object1.getVelocity();
	Vector acc = m_object1.getAcceleration();
	MyPhoton::getInstance().sendMyData(pos.mVal[0], pos.mVal[1],
										vel.mVal[0], vel.mVal[1],
										acc.mVal[0], acc.mVal[1]);
}

void Application::networkUpdate()
{
	static double prevTime = glfwGetTime();
	
	double time = glfwGetTime();
	if(time - prevTime >= gNetworkFrameTime) 
	{
		sendMyData();
		prevTime = time;
	}
}

void Application::limitVelAndPos(GameObject* go)
{
	if(go->getVelocity().length() > 400.0f)
	{
		Vector vec = go->getVelocity();
		vec.normalize();
		vec *= 400.0f;
		go->setVelocity(vec);
	}
}

void Application::update (double elapsedTime)
{

	MyPhoton::getInstance().run();

	if(!m_gameStarted)
		return;


	m_object1.update(elapsedTime);
	m_object1.setAcceleration(Vector(0.0f, 0.0f, 0.0f));
	limitVelAndPos(&m_object1);
	
	networkUpdate();

	if(m_method == 0)
	{
		m_object2.setPos(m_lastReceivedPos);
	}
	else if(m_method == 1)
	{
		m_object2.setPos(m_object2.getPos()*0.96f + m_lastReceivedPos*0.04f);
	}

	else if(m_method == 2)
	{
		m_object2.update(elapsedTime);
		m_object2.setAcceleration(Vector(0.0f, 0.0f, 0.0f));
		limitVelAndPos(&m_object2);
	}
	else if(m_method == 3)
	{
		m_object2.update(elapsedTime);

		//very slowly interpolate from on-going predicting pos to lastest received pos. Without this interpolation, the offset of opponent position will keep being accumulated. 
		m_object2.setPos(m_object2.getPos()*0.995f + m_lastReceivedPos*0.005f); 
		
		limitVelAndPos(&m_object2);
	}
}

void Application::draw()
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if(!m_gameStarted)
		return;

	m_object1.draw();
	m_object2.draw();
}


void Application::OnReceivedOpponentData(float* data)
{
	if(m_gameStarted == false)
	{
		m_gameStarted = true;
		m_object2.setPos(Vector(data[0], data[1], 0));

		m_lastReceivedPos = m_object2.getPos();
		m_prevReceivedTime = glfwGetTime();
		return;
	}

	if(m_method == 0 || m_method == 1)
	{
		m_lastReceivedPos = Vector(data[0], data[1], 0);
	}
	else if(m_method == 2)
	{
		double currentTime = glfwGetTime();
		double timeSinceLastReceive = currentTime - m_prevReceivedTime;

		if(timeSinceLastReceive >= gNetworkFrameTime*0.5f) //filter the noise
		{
			Vector targetPos = Vector(data[0], data[1], 0.0f);

			float length = (m_lastReceivedPos - targetPos).length() / timeSinceLastReceive;
			Vector goVec = targetPos - m_object2.getPos();
			goVec.normalize();

			goVec = goVec * length;

			Vector finalVel = m_object2.getVelocity()*0.6f + goVec *0.4f;
			m_object2.setVelocity(finalVel);
		
			m_lastReceivedPos = targetPos;
			m_prevReceivedTime = currentTime;
		}
	}
	else if(m_method == 3)
	{
		m_lastReceivedPos = Vector(data[0], data[1], 0);
		m_object2.setVelocity(Vector(data[2], data[3], 0));
		m_object2.setAcceleration(Vector(data[4], data[5], 0));
	}
}


void Application::onKeyPressed (int key)
{

}

void Application::onKeyReleased (int key)
{
}

void Application::onKeyHold (int key)
{
	if(!m_gameStarted)
		return;
	
	if (key == GLFW_KEY_W)
	{
		m_object1.setAcceleration(m_object1.getAcceleration()+Vector(0.0f, 500.0f, 0.0f));
	}
	if(key == GLFW_KEY_A)
	{
		m_object1.setAcceleration(m_object1.getAcceleration()+Vector(-500.0f, 0.0f, 0.0f));
	}
	if(key == GLFW_KEY_S)
	{
		m_object1.setAcceleration(m_object1.getAcceleration()+Vector(0.0f, -500.0f, 0.0f));
	}
	if(key == GLFW_KEY_D)
	{
		m_object1.setAcceleration(m_object1.getAcceleration()+Vector(500.0f, 0.0f, 0.0f));
	}
	
	
	/*if(key == GLFW_KEY_W)
	{
		m_object1.setVelocity(Vector(0.0f, 400.0f, 0.0f));
	}
	if(key == GLFW_KEY_A)
	{
		m_object1.setVelocity(Vector(-400.0f, 0.0f, 0.0f));
	}
	if(key == GLFW_KEY_S)
	{
		m_object1.setVelocity(Vector(0.0f, -400.0f, 0.0f));
	}
	if(key == GLFW_KEY_D)
	{
		m_object1.setVelocity(Vector(400.0f, 0.0f, 0.0f));
	}
	*/
}

void Application::onMousePressed (int button)
{

}

void Application::onMouseReleased (int button)
{

}

void Application::onMouseHold (int button)
{

}

void Application::onMouseMoved (double mousePosX, double mousePosY)
{
	
}
