#include "ai_client.hpp"

ai_client::ai_client()
{
}

ai_client::~ai_client()
{
}

void ai_client::accessed()
{
    this->lastAccessed = std::chrono::steady_clock::now();
}
