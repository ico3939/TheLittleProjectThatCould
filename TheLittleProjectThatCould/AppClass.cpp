#include "AppClass.h"
using namespace Simplex;
void Application::InitVariables(void)
{
	////Set the position and target of the camera
	//m_pCameraMngr->SetPositionTargetAndUp(
	//	vector3(0.0f, 0.0f, 13.0f), //Position
	//	vector3(0.0f, 0.0f, 12.0f),	//Target
	//	AXIS_Y);					//Up

	m_pLightMngr->SetPosition(vector3(0.0f, 3.0f, 13.0f), 1); //set the position of first light (0 is reserved for ambient light)

															  //Entity Manager
	m_pEntityMngr = MyEntityManager::GetInstance();

	//creeper
	m_pEntityMngr->AddEntity("Minecraft\\train2.obj", Tag::Player, "Creeper");
	m_pEntityMngr->SetAxisVisibility(true, "Creeper"); //set visibility of the entity's axis
	m_pEntityMngr->UsePhysicsSolver();
	m_pEntityMngr->SetModelMatrix(glm::translate(vector3(0.0f, 0, 0.0f)));

	for (int i = 0; i < 100; i++) {
		//TODO make this work with the tree obj
		m_pEntityMngr->AddEntity("Minecraft\\Steve.obj", Tag::Passenger, "Steve");
		matrix4 mSteve = glm::translate(vector3(rand() % 99 - 50, 0, rand() % 99 - 50));
		m_pEntityMngr->SetModelMatrix(mSteve);
		//m_pEntityMngr->UsePhysicsSolver();
	}

	for (int i = 0; i < 100; i++) {
		//TODO make this work with the tree obj
		m_pEntityMngr->AddEntity("Minecraft\\evergreen.obj", Tag::Passenger, "Tree");
		matrix4 mTree = glm::translate(vector3(rand() % 99 - 50, 0, rand() % 99 - 50));
		m_pEntityMngr->SetModelMatrix(mTree);
		//m_pEntityMngr->UsePhysicsSolver();
	}

	m_pEntityMngr->AddEntity("Minecraft\\Cube.obj", Tag::Floor, "Ground");
	m_pEntityMngr->SetModelMatrix(glm::translate(vector3(-50.0f, -1.0f, -50.0f)) * glm::scale(vector3(100.0f, 1.0f, 100.0f)));


	/*planeIdx = m_pMeshMngr->GeneratePlane(100.0f, vector3(0.0f, .5f, 0.0f));
	planeMatrix = glm::translate(vector3(0.0f, 0.0f, 0.0f)) * ToMatrix4(glm::quat(vector3(-PI / 2.0f, 0.0f, 0.0f)));*/

	vector3 creeperPos = m_pEntityMngr->GetRigidBody("Creeper")->GetCenterGlobal();
	vector3 cameraPos = creeperPos + vector3(0, 2, -5);
	creeperPos.y += 1;
	m_pCameraMngr->SetPositionTargetAndUp(cameraPos, creeperPos, AXIS_Y);

	m_uOctantLevels = 3;

	m_pRoot = new MyOctant(m_uOctantLevels, 5);

	m_pEntityMngr->Update();
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the first person camera active?
	CameraRotation();

	//Update Entity Manager
	m_pEntityMngr->Update();

	//update things for the creeper (rotation and dimensions)
	matrix4 mCreeper = m_pEntityMngr->GetModelMatrix("Creeper") * ToMatrix4(m_qCreeper) * ToMatrix4(m_qArcBall);
	m_pEntityMngr->SetModelMatrix(mCreeper, "Creeper");
	m_pRoot->UpdateIdForEntity(0);

	//m_pEntityMngr->SetModelMatrix(m_pEntityMngr->GetModelMatrix("Ground") * glm::scale(vector3(100.0f, 2.0f, 100.0f)), "Ground");

	//rotate camera around creeper
	vector3 creeperPos = m_pEntityMngr->GetRigidBody("Creeper")->GetCenterGlobal();
	float x = sin(cameraAngle);
	float y = cos(cameraAngle);
	vector3 cameraPos = creeperPos + vector3(-5 * (x), 2, -5 * (y));
	creeperPos.y += 1;
	m_pCameraMngr->SetPositionTargetAndUp(cameraPos, creeperPos, AXIS_Y);

	//Add objects to render list
	m_pEntityMngr->AddEntityToRenderList(-1, true);
	m_pMeshMngr->AddMeshToRenderList(m_pMeshMngr->GetMesh(planeIdx), planeMatrix);



}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	if (m_uOctantID == -1)
		m_pRoot->Display();
	else
		m_pRoot->Display(m_uOctantID);

	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList();

	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();

	//draw gui
	DrawGUI();

	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}
void Application::Release(void)
{
	SafeDelete(m_pRoot);

	//release the entity manager
	m_pEntityMngr->ReleaseInstance();

	//release GUI
	ShutdownGUI();
}
