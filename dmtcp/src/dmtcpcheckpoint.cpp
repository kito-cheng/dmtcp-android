/***************************************************************************
 *   Copyright (C) 2006 by Jason Ansel                                     *
 *   jansel@ccs.neu.edu                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <stdio.h>
#include "jassert.h"
#include "jfilesystem.h"
#include "jconvert.h"
#include "constants.h"
#include <errno.h>

static std::string _stderrProcPath() 
{
     return "/proc/" + jalib::XToString( getpid()) + "/fd/" + jalib::XToString(fileno(stderr));
}

int main(int argc, char** argv)
{

    //setup hijack library
    std::string dmtcphjk = jalib::Filesystem::FindHelperUtility( "dmtcphijack.so" );
    std::string searchDir = jalib::Filesystem::GetProgramDir();
    const char* ckptDir = getenv("PWD");
    if(ckptDir == NULL) ckptDir = ".";
     
    JASSERT(argc >= 2)(argc).Text("Usage: dmtcp_checkpoint ./cmd ...");
    
    
    std::string stderrDevice = jalib::Filesystem::ResolveSymlink( _stderrProcPath() );
    
    //TODO:
    // When stderr is a socket, this logic fails and JASSERT may write data to FD 3 
    // this will cause problems in programs that use FD 3 for algorithmic things...
    if( stderrDevice.length() > 0 
     && jalib::Filesystem::FileExists(stderrDevice))
        setenv(ENV_VAR_STDERR_PATH,stderrDevice.c_str(), 0);
    
    setenv("LD_PRELOAD", dmtcphjk.c_str(), 1);
    setenv(ENV_VAR_HIJACK_LIB, dmtcphjk.c_str(), 0);
    setenv(ENV_VAR_UTILITY_DIR, searchDir.c_str(), 0);
    setenv(ENV_VAR_CHECKPOINT_DIR, ckptDir, 0);
    
    //how many args to trim off start
    int startArg = 1;
    
    //copy args into new structure
    char** newArgs = new char* [argc];
    memset( newArgs, 0, sizeof(char*)*argc );
    for(int i=0; i<argc-startArg; ++i)
        newArgs[i] = argv[i+startArg];
    
    //run the user program
    execvp(newArgs[0], newArgs); 
    
    //should be unreachable
    JASSERT(false)(newArgs[0])(JASSERT_ERRNO).Text("exec() failed");
    
    return -1;
}
