#pragma once
#include <iostream>
class Node
{
private:
	
		int y;
		int x;
		int parentX;
		int parentY;
		float gCost;
		float hCost;
		float fCost;
		bool obstacle;
		bool start;
		bool stop;
		bool normal;

public:
		void assignNodestate(int a, int i, int j ,bool jump)
		{
			if (jump == 1) std::cout << std::endl; 
			if (a == 0) obstacle = 1;
			else if (a == 1) start = 1;
			else if (a == 2) stop = 1;
			else normal = 1;
			std::cout << a << " ";

		}
		int calculateHcost() {

		}
		int calculateGcost() {

		}
		int calculateFcost() {

		} 
		void changeParentnode() {

		}


};

