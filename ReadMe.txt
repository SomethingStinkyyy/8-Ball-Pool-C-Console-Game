
In order to execute the game code in the console itself, please follow the following steps, 

1. The Game Engine Header File requires a compiler that can handle unicode, so the code should be opened in Visual Studio. 
2. Create a blank C++ project and right click on the project solution in the solution explorer. Choose add new item. 
3. From the pop up window, choose a C++ file and add it to the project. 
4. Copy and paste the C++ code into this file. 
5. Now locate the folder containing the Visual Studio Project, and copy the GameEngine.h header file as well as the 
pixelfont.spr files. Make sure to paste this in the Project Folder, inside of the Visual Studio Project Folder containing
the previous C++ code (To access the header file and the font file the folder of the code must be the same).
5. Then, right click on the solution explorer and choose add existing item. Select the GameEngine.h header file from the
Visual Studio Solution Folder. 
6. Now debug the code in x86 in VS and the game should play. (In the event of the worst case scenario, a built executive file of 
the game is available in the Built Game folder). 