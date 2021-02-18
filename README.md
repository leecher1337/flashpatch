# What is it?
Patches Adobe Flash Player timebomb on Windows.
In Adobe Flash Player versions newer than 32.0.0.344 they added a "Timebomb" for the EOL. the player would refuse to run any custom flash content after 12/01/2021,

# Why another patcher?
When looking for a solution for the problem, I found this analysis of the timebomb:
https://gist.github.com/KuromeSan/56d8b724c0696b54f9f81994ae3591d1

Unfortunately, I only found patchers for the timebomb that were written in .NET.
Now I definitely don't want to pollute my system with .NET Framework and the patcher above didn't even work properly on Windows XP.
So I decided to write my own little patcher that is just a tiny executable wihout any dependencies.
It is based on the work mentioned above.

# How to use?
Depending on whether you are on x86 or x64 System, download flashpatch_x86.exe or flashpatch_x64.exe from Releases-page.
If you just want to look in the default locations and patch Flash installation there, simply run the .exe
If you want to scan a certain directory or patch a specifig file, specify it as a commandline parameter.

