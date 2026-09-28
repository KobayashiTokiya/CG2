#pragma once
class ScoreManager
{
public:
	static void SetScore(int score) { score_ = score; }
	static int GetScore() { return score_; }
private:
	static inline int score_ = 0;
};

