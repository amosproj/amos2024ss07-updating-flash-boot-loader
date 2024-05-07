# How to execute:

This is a simple c Programm, just use the IDE of your choice, build and execute the files.

##ATTENTION: possible issues

On my end I had to link the IDE to the "comdlg32" library in the project settings. I wasn't able to do this in VSC, because I couldn't find the option for that.

That's why I switched to Code::Blocks, a horrible IDE but it got the job done :')

It is easy to install and to set up.

### Setup project:

1) First install Code::Blocks and create a new project inside the 06_binary_converter. (my .gitignore will ignore all files that will be created by this, no worries) 

2) On the left side will be your workspace and the newly created project, right click the project name and select "Add files...".

3) select all files in there (main.c, converter.c & converter.h) and add them to the project.

4) You can now build the project and run it. If you encounter a problem with the "GetOpenFileName" function, you will have to follow the steps below

### Linking "comdlg32"

1) Right click the name of the project on the left hand side

2) Select "Build options..."

3) On the left side of the new window you will see 3 options: <project_name>, Debug and Release. Not sure for which it has to be set but I just did it for all 3 of them. I suspected that doing it for <project_name> should cover all but I sometimes had to redo this…

For each option you will have to select the Tab "Linker settings" in the middle and click on "Add" on the left side for the "Link libraries" section.

4) Add the library "comdlg32" without quotation marks

5) After doing this you can try to rebuild and run the project. It should work now … hopefully :)
