#pragma once
#include "Sprite.h"
#include "GameObject.h"

#include <list>

class Application
{
protected:
	int m_method;

	Sprite*		m_sprite01;
	GameObject	m_object1;
	GameObject	m_object2;

	Vector m_lastReceivedPos;
	double m_prevReceivedTime;
	bool m_gameStarted;

	void networkUpdate();

	void limitVelAndPos(GameObject* go);

public:
	
	Application();
	~Application();


	static Application& getInstance()
	{
		static Application app;
		return app;
	}

	void sendMyData(void);
	void OnReceivedOpponentData(float* data);

	virtual void start();	
	virtual void update(double elapsedTime);
	virtual void draw();
	virtual void onKeyPressed(int key);
	virtual void onKeyReleased(int key);
	virtual void onKeyHold(int key);
	virtual void onMousePressed(int button);
	virtual void onMouseReleased(int button);
	virtual void onMouseHold(int button);
	virtual void onMouseMoved(double mousePosX, double mousePosY);
};

