# radcommlink
############################################################################

 Copyright (C) 2019 GrizzWorks, LLC
 ALL RIGHTS RESERVED
 This file is part of radcommlink software project under the GNU GPLv3
 license.

 radcommlink is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 Parts of radcommlink may contain source code from Open Source projects.
 See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
############################################################################

A Qt client layered above a working installation of PAT used for two-way radio message forwarding.  See: https://getpat.io

This project is a framework for a client which works alongside the excellent cross-platform PAT Winlink client.

PAT has a command line interface and a web interface.  These may be considered inconvenient for some, so a thick UI client has been created to fill that void.  The thick client also allows interoperability with ancillary systems which might communicate I/O data or application telemetry via different methods.

The project is written using the Qt5 framework, currently 5.12.3, but the project should compile and run even in Qt4 (untested).

The Qt libraries qserialport and qnetwork are required along with qtbase and all other basic libraries for Qt functionality. (Debian and derivatives use: sudo apt install qt5base qt5serialport5-dev qt5network5-dev).  A .pro file is included for use with current versions of the QtCreator IDE.

The basic process for using radcommlink is to install PAT, configure it to your liking and ensure that it all works peroperly from the command line.  Once this is accomplished, starting radcommlink should be able to "talk to" and interact with the PAT installation in order to take advantage of it's services.

The user may create WINLINK style messages into the PAT mailbox structure directly using a simple UI form.  The created or received message files are displayed in the UI for the user to view and delete.  A PAT command line is made available if the user wishes to interact with PAT directly, and the replies from PAT are displayed in a text area to show messaging progress, errors, and other information.  

The user may update and display the Station List from the UI.  The connection controls allow the user to choose the remote station, the transport (ax25,ardop,winmor,pactor,telnet).  Pactor transport functionality is part of the development branch of PAT at the moment (2019-09). The user may choose a "Radio only" connection, and choose the dial frequency if radio control is configured.

NOTE: the .wl2k folder MUST be in the current user's home directory.  If the user has installed PAT in an alternate manner, then a symlink should be created from that location to ~/.wl2k in order for radcommlink to function properly.

This project was initiated on Linux and is geared to those who wish to use Winlink but do not wish to use Windows or Winlink Express.  While it should function fine with a Windows based PAT installation, it has not been tested.  Observations and expericnes on Windows are welcome.
