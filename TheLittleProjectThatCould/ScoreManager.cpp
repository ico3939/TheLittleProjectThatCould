#include "ScoreManager.h"


using namespace Simplex;

uint ScoreManager::m_uScore = 0;

ScoreManager::ScoreManager()
{
}


ScoreManager::~ScoreManager()
{
}

uint ScoreManager::GetScore()
{
	return m_uScore;
}

void ScoreManager::IncreaseScore(uint amount)
{
	m_uScore += amount;
}

void ScoreManager::ResetScore()
{
	m_uScore = 0;
}
