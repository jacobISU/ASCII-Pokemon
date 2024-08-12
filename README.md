# ASCII Pokemon

## About
This was a fun project for my Advanced Programming Techniques class (CS 327) in college using C and C++.  

**Disclosure**: All code in the final project is not my own, as it was a semester project with checkpoint milestones. However, all features and data structures were implemented by me throughout the project.

## Running Game WITH Visual Studio Code.
I used Visual Studio Code and MinGW to run this project on Windows. It was originally run on Unix, but I changed it to run on Windows for convenience.

### Step 1: Install MinGW
Download MinGW (a native Windows port of the GNU Compiler Collection (GCC)) from [here](https://sourceforge.net/projects/mingw/). You need this because you will need to install the following libraries from it to run the program on Windows:
- `libstdc++-6.dll`: Provides the standard C++ library implementations, such as containers, algorithms, and utilities.
- `libgcc_s_dw2-1.dll`: Provides exception handling and other runtime functionalities.
- `libncursesw-6.dll`: Used for creating text-based user interfaces in terminal applications.

### Step 2: Build and Run
1. Open the "ascii-pokemon" folder in VSCode.
2. Open a terminal and navigate to the `smith_jacob.assignment-1.10` directory:
   ```bash
   cd smith_jacob.assignment-1.10
   make
   .\poke327.exe
   ```
   This will run the game in the terminal.

   > Note: If terminal is too small, game will be cut off. Resize terminal window and restart. 

### Step 3: See README in "Play ASCII Pokemon" folder for controls. 

## Running Game WITHOUT Visual Studio Code.
The MinGW libraries are essential in running this program.

### Step 1: Download "Ascii-pokemon.zip" file in the "Play ASCII Pokemon" folder or just download the whole folder. 
Both contain the pokemon database, poke327.exe file, and README.

### Step 2: Install MinGW
Download MinGW (a native Windows port of the GNU Compiler Collection (GCC)) from [here](https://sourceforge.net/projects/mingw/). You need this because you will need to install the following libraries from it to run the program on Windows:
- `libstdc++-6.dll`: Provides the standard C++ library implementations, such as containers, algorithms, and utilities.
- `libgcc_s_dw2-1.dll`: Provides exception handling and other runtime functionalities.
- `libncursesw-6.dll`: Used for creating text-based user interfaces in terminal applications.

### Step 3: Start poke327.exe file.
If the libraries haven't been installed then a window will pop up telling you to install them first.
Otherwise, a terminal will pop-up with "Loading Game..." and the game will start.

Terminal.
![image](https://github.com/user-attachments/assets/7716106c-74a3-4796-be6c-7b91f0c2f937)

Opening Screen.
![image](https://github.com/user-attachments/assets/06da49f8-1719-4dbb-a54c-6c03ebfda540)



  


