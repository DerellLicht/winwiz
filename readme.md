### WinWiz - a win32 port of the ancient DOS game Wizard's Castle

Notes on building this application:

1. I use MinGW 32-bit to build it, I recommend that you do so as well

2. Clone the develop branch, not master; someday I really *do* need to merge back into master,
   but I haven't done so yet...

3. Use --recursive when cloning the repository, so that you end up with the der_libs module as well.

So the clone should look like this:

git clone -b develop https://github.com/DerellLicht/winwiz.git --recursive

