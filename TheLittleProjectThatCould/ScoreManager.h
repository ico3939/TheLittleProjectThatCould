#ifndef __SCOREMANAGER_H_
#define __SCOREMANAGER_H_

#include "Definitions.h"

namespace Simplex {


	class ScoreManager
	{
	private:
		static uint m_uScore;
		ScoreManager();
		~ScoreManager();
	public:
		static uint GetScore();
		static void IncreaseScore(uint amount);
		static void ResetScore();
	};

}


#endif
