#include "stdafx.h"
#include "Server.h"

int main()
{
	CServer Server{};

	while (true)
	{
		int i{};
		
		cout << "1을 입력하면 메인스레드가 종료됩니다." << endl;
		cin >> i;
	}
}
