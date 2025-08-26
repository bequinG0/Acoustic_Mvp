#include "RedisPublisher.h"
#include "Triangulator.h"
#include <vector>

class TriangulationTask
{
    private:
        RedisPublisher publisher;
        Triangulator triangulator;
        std::vector <double> eventsData;
    public:
        void execute()
        {

        }
};