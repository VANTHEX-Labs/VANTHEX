#include <iostream>

#include "NetworkScanner.h"
#include "UdpCore/UdpScanner.h"
#include "TcpCore/TcpScanner.h"


#define RESET   "\033[0m"
#define GREEN   "\033[38;2;80;220;150m"
#define ORANGE  "\033[38;2;255;165;0m"
#define RED     "\033[38;2;255;80;100m" // Ошибка
#define CYAN	"\033[36m"
#define PURPLE  "\033[38;2;160;90;255m" // основной цвет
#define GREY    "\033[90m"

void NetworkScanner()
{
	std::cout << "" << std::string(103, '-') << "\n";
    std::cout << PURPLE;
    std::cout << R"(
     //.       0/ 0000000000. 00000000000  00      00      00    00000000    000000000/   /01     100. 
     0000     101 100........ /..../00.... 100    0000    00'  /00......:00  /00......00 :/00   :00:   
     00100    001 /001            :00       00   :0''0.   00  :00/      ::00 /00     :00  /00 :00/     
    .011 00   001 /000000000/     100       .0/  00/ 00  00'  00/        :00 /000000000.  /0000/00     
    0011  00: 001 /00........     001        00 '0'  :0/ 00   '00         00 /00......00  /00/.::000   
    0011   00'001 001'            001         0000.   0000'   .000       001 /00     :/00 /00:  :::00  
    0011    00001 00000000000     001         000'    .000     ::000000000/  :00      /00 /100    ::100
    ..1.     ...1 ...........1    ..1          ..      ..:       ........    :..      :.. ...      ....
)";
	std::cout << RESET;
	std::cout << "" << std::string(103, '-') << "\n\n";
    std::cout << PURPLE << "     [1] " << RESET  << " Tcp Scanner\n";
    std::cout << PURPLE << "     [2] " << RESET  << " Udp Scanner\n\n";
	std::cout << "" << std::string(103, '-') << "\n";
	std::cout << ORANGE << "     [0] " << RESET  << " Back to main menu\n" << RESET;

    std::cout << PURPLE << "     NETWORK SCANNER" << GREY << " :: "<< RESET << " Select module > " << ORANGE;
	int choice = -1;

    if (!(std::cin >> choice)) {
		std::cout << RED << "[-] Ошибка. Ввод не был прочитан\n" << RESET;
		std::cin.clear();
		std::cin.ignore(1000000, '\n');
		return;
    }
    switch (choice) {
	case 1:
		TcpScanner();
		break;
	case 2:
		UdpScanner();
		break;
	case 0:
		std::cout << RESET;
		return;
	default:
		std::cout << RED << "[-] Ошибка. Неверный выбор\n" << RESET;
		std::cin.clear();
		std::cin.ignore(1000000, '\n');
    }
    return;
}
