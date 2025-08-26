#include <iostream>
#include <hiredis/hiredis.h>

using namespace std;

bool test(const char* host)
{
  return true;
}

int main()
{
  redisContext* context = redisConnect("localhost", 6389);
  if(context == nullptr) 
  {
    cout << "[ERR] Ошибка подключения к redis" << "\n";
    return 1;
  }

  cout << "[**] Удачное подключение к redis" << "\n";

  redisFree(context);


  const char* temp = "asd";
  cout << test << "\n";
}
