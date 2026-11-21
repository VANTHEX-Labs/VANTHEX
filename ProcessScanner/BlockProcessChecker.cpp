#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <fstream>
#include <chrono>

#include "ProcessScanner.h"
#include "ProcessCheckerPID/ProcessCheckerPID.h"
#include "ProcessCheckerALL/ProcessCheckerALL.h"
#include "ProcessCheckerName/ProcessCheckerName.h"


#define RESET   "\033[0m"
#define PURPLE  "\033[38;2;160;90;255m" // основной цвет
#define GREY    "\033[90m"
#define GREEN   "\033[38;2;80;220;150m"
#define ORANGE  "\033[38;2;255;165;0m" 
#define CYAN	"\033[36m"
#define RED     "\033[38;2;255;80;100m" // ошибка


void ShowMenuProcess()
{

	std::cout << PURPLE;
	std::cout << R"(
  @@@@@@@@@@.   @@@@@@@@@@@+        =@@@@@@@@=         @@@@@@@@@    -@@@@@@@@@@@@@   @@@@@@@@@=     =@@@@@@@@@    
 +@@@@@@@@@@@@+ @@@@@@@@@@@@@@    @@@@@@@@@@@@@@     @@@@@@#@@@@@@@ -@@@@@@@@@@@@@ =@@@@*--+@@@@=  @@@@@=.-@@@@@  
 @@@@#.    @@@@+@@@@-     @@@@#  @@@@*      =@@@@  -@@@@+      +@@@@-@@@@          #@@@     -@@@@ -@@@@    .-+++= 
 @@@@*+=   @@@@+@@@@+.   +@@@@= @@@@#=.     .#@@@@ @@@@+-     .-    -@@@@@@@@@@@@ --@@@@@@@@+=   --.@@@@@@@@@=    
 @@@@@@@@@@@@@=*@@@@@@@@@@@@=.=+@@@@+=       #@@@@ @@@@+            -@@@@@@@@@@@@*-.  .+@@@@@@@@@-..  .+@@@@@@@@@ 
-@@@@@@@@@@+. -@@@@*....=@@@@#- @@@@#        @@@@# @@@@@        @@@@-@@@@         ..          @@@@.=+++       @@@@
+@@@#+         @@@@*-    @@@@#  .@@@@@      @@@@@= -@@@@@      @@@@@-@@@@         .-@@@@@    =@@@@--@@@@@   -+@@@@
@@@@*++.       @@@@++    @@@@#+  .@@@@@@@@@@@@@+-= -.=@@@@@@@@@@@@*=-#@@@@@@@@@@@@@-.=@@@@@@@@@@@=--.+@@@@@@@@@@@.
@@@@*++        @@@@++    =@@@@+   .  #@@@@@@+   -   ..  =@@@@@*-  . -++----------....   .+@@@*-   ..    .++++*-   
     =+             +                                               -                                             
)";

}



void ProcessScanner()
{
	ShowMenuProcess();
	
	setlocale(LC_ALL, ".UTF8");

	std::cout << RESET;
	std::cout << "" << std::string(103, '-') << "\n\n";
	std::cout << PURPLE << "     [1] " << RESET << " Process Scanner (ALL)\n";
	std::cout << PURPLE << "     [2] " << RESET << " Process Scanner (need PID)\n";
	std::cout << PURPLE << "     [3] " << RESET << " Process Scanner (need Name)\n\n";
	std::cout << "" << std::string(103, '-') << "\n";
	std::cout << ORANGE << "     [0] " << RESET << " Back to main menu\n" << RESET;

	std::cout << PURPLE << "     PROCESS SCANNER" << GREY << " :: " << RESET << " Select module > " << ORANGE;
	int choice = -1;

	if (!(std::cin >> choice)) {
		std::cout << RED << ">> [-] Ошибка. Ввод не был прочитан\n" << RESET;
		std::cin.clear();
		std::cin.ignore(1000000, '\n');
		return;
	}
	std::cout << RESET;

	switch (choice) {

	case 1:
		ProcessCheckerALL();
		break;

	case 2:
		ProcessCheckerPID();
		break;

	case 3:
		ProcessCheckerName();
		break;

	case 0:
		std::cout << RESET;
		return;

	default:
		std::cout << RED << ">> [-] Ошибка. Неверный выбор\n" << RESET;
		std::cin.clear();
		std::cin.ignore(1000000, '\n');
	}
	return;
}