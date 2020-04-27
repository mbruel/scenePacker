# scenePacker v1.0

**Command line / GUI tool that packs every files/folders of the input folders individually using RAR**<br/>
<br/>
![scenePacker](https://raw.githubusercontent.com/mbruel/scenePacker/master/pics/scenePacker_v1.0.png)
<br/>
The archives will either be in ONE destination folder or inside a 'rar subfolder' of each inputs<br/>
you can:
  - generate a random name for the archives and set its length
  - generate a random password or use a fixed one for all the archives
  - split the archives into several volumes
  - lock the archive (-k Rar option)
  - add recovery records (-rr Rar option)
  - set the compression level (from 0 to 5)

An history log will be maintained or you can generate a separate log for each execution using the config **logPerRun**.
<br /><br />
I've built only a Win32 release on Windows7. It should be compatible with all versions of Windows (from win7)<br/>
You can either use [the installer](https://github.com/mbruel/scenePacker/releases/download/v1.0/scenePacker_v1.0_x64_setup.exe) or [the portable version](https://github.com/mbruel/scenePacker/releases/download/v1.2/scenePacker_v1.2_win64.zip)<br/>
<br />

### How to build
scenePacker is developped in **C++11 / Qt5** <br/>

#### Dependencies:
- build-essential (C++ compiler, libstdc++, make,...)
- qt5-default (Qt5 libraries and headers)
- qt5-qmake (to generate the moc files and create the Makefile)

#### Build:
- go to the src folder
- qmake
- make

Easy! it should have generate the executable **scenePacker**</br>
you can copy it somewhere in your PATH so it will be accessible from anywhere

### How to use it in command line
<pre>
Syntax: scenePacker (options)* (-i <src_folder>)+ (-o <dst_path>| -x <rar_folder>)
	-h or --help       : Help: display syntax
	-v or --version    : app version
	-i or --input      : source folder (can use several -i)
	-o or --dstPath    : destination folder
	-x or --rarFolder  : name of the rar folder where the archives will be created within each source folder
	-d or --debug      : display debug information
	-t or --threads    : number of threads (compression in parallel)
	-p or --pass       : use a fixed password for all archives
	-s or --volSize    : split archive into volume of that size (in MB)
	-r or --recPct     : percentage of recovery records to add to the archives (Winrar -rr option)
	-l or --lock       : lock archives (Winrar -k option)
	--genName          : generate random name for each archive
	--genPass          : generate random password for each archive
	--lengthName       : length of the random name
	--lengthPass       : length of the random password

Examples:
  1.: using dst path:   scenePacker -i ~/Downloads/folder1 -i ~/Downloads/folder2 -o /tmp/archives --genName --genPass --lengthPass 17
  2.: using rar folder: scenePacker -i ~/Downloads/folder1 -i ~/Downloads/folder2 -x _rar

The first example will create all archives from both folder1 and folder2 inside the destination folder /tmp/archives
The second example, will create a '_rar' folder in both folder1 et folder2 with their respective archives
</pre>


### Licence
<pre>
//========================================================================
//
// Copyright (C) 2020 Matthieu Bruel <Matthieu.Bruel@gmail.com>
//
//
// scenePacker is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; version 3.0 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301,
// USA.
//
//========================================================================
</pre>


### Questions / Issues / Requests
- if you've any troubles to build or run scenePacker, feel free to drop me an email
- if you've some comments on the code, any questions on the implementation or any proposal for improvements, I'll be happy to discuss it with you so idem, feel free to drop me an email
- if you'd like some other features, same same (but different), drop me an email ;)

Here is my email: Matthieu.Bruel@gmail.com



### Donations
I'm Freelance nowadays, working on several personal projects, so if you use the app and would like to contribute to the effort, feel free to donate what you can.<br/>
<br/>
<a href="https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=W2C236U6JNTUA&item_name=scenePacker&currency_code=EUR"><img align="left" src="https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif" alt="scenePacker"></a>
 or in Bitcoin at this address: **3BGbnvnnBCCqrGuq1ytRqUMciAyMXjXAv6**
<img align="right" align="bottom" width="120" height="120" src="https://raw.githubusercontent.com/mbruel/ngPost/master/pics/btc_qr.gif" alt="scenePacker">
