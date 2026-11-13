#include <iostream>
#include <limits>
#include <windows.h>

#include "ProcessScanner/ProcessScanner.h"
#include "MemoryScanner/MemoryScanner.h"
#include "NetworkScanner/NetworkScanner.h"
#include "PathScanner/PathScanner.h"

#define RESET   "\033[0m"
#define PURPLE  "\033[38;2;160;90;255m" // основной цвет
#define WHITE   "\033[97m"
#define GREY    "\033[90m"
#define GREEN   "\033[38;2;80;220;150m"
#define ORANGE  "\033[38;2;255;165;0m" 
#define RED     "\033[38;2;255;80;100m" // ошибка



void ShowMenu()
{

    std::cout << PURPLE;
    std::cout << R"(
    ##        %##      -##.      @##        ## ############ ##        ### ########### -##@      ###   
    @##      +##%%    +####      ####*      ## .....###.... ###       %##:###........ ---##    ###    
     ###    +##@%%   +##%+##     ###=##    *## .....###.... ###       *##*%##........ ----=## ##@     
     @##.  +##@%%   +##%% ##*    ###--##   +##      ###     ############# *##########   ----###=      
      ### +##@%%   +#########    ##@ --##* ###      ###     ###.......*## *##........    .-#####+     
      @##*##%%%   @##......###   ##@   --#####      ###     ###.......*## *##........     ###--:##    
       ####%%%   @##%%......###  ##@    --####      ###     ###       *## *############ .##% ----%##  
        ..%%%    ..%%=      -..  ..@     --...      ...     ...       *.. *...............=    ---....
         ..%     ...=        ... ..@       ...      ...     ...       +.. ............ ...      -....  
)";

    std::cout << RESET;
    std::cout << GREY << "     " << std::string(102, '=') << "\n";
    std::cout << WHITE << "                       SYSTEM ANALYSIS CONSOLE\n";
    std::cout << GREY << "     " << std::string(102, '=') << "\n";

    std::cout << PURPLE << "     [1] " << WHITE << "Process Scanner\n";
    std::cout << PURPLE << "     [2] " << WHITE << "Memory Scanner (PID)" << "\n";
    std::cout << PURPLE << "     [3] " << WHITE << "Network Scanner (PID or Name)" << "\n";
    std::cout << PURPLE << "     [4] " << WHITE << "Path Scanner (PID)" << "\n\n";
    std::cout << GREY << "     " << std::string(102, '-') << "\n";
    std::cout << ORANGE << "     [0] " << WHITE << "Exit VANTHEX\n";
    std::cout << GREY << "     " << std::string(102, '-') << "\n\n";

    std::cout << PURPLE << "     VANTHEX" << GREY << " :: " << WHITE << "Select module > " << ORANGE;
    
}

int main()
{
    setlocale(LC_ALL, ".UTF8");

    int choice = -1;

    while (true)
    {
        ShowMenu();
        std::cin.clear();
        if (!(std::cin >> choice))
        {
            std::cout << RESET;
            std::cin.clear();
            std::cout << RED << "\n     [-] Invalid input!\n" << RESET;
            std::cin.ignore(10000, '\n');
            system("pause");
            continue;
        }

        switch (choice)
        {
        case 1:
            std::cout << RESET;
            ProcessScanner();
            break;

        case 2:
            std::cout << RESET;
            MemoryScanner();
            break;

        case 3:
            std::cout << RESET;
            NetworkScanner();
            break;
        case 4:
            std::cout << RESET;
            PathChecker();
            break;
        case 0:
            std::cout << RESET;
            std::cout << GREEN << "\n     [+] VANTHEX shutting down...\n" << RESET;
            return 0;

        default:
            std::cout << RESET;
            std::cout << RED << "\n     [-] Unknown option!\n" << RESET;
            system("pause");
            break;
        }
    }
}