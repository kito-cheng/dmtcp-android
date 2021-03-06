/****************************************************************************
 *   Copyright (C) 2006-2008 by Jason Ansel, Kapil Arya, and Gene Cooperman *
 *   jansel@csail.mit.edu, kapil@ccs.neu.edu, gene@ccs.neu.edu              *
 *                                                                          *
 *   This file is part of the dmtcp/src module of DMTCP (DMTCP:dmtcp/src).  *
 *                                                                          *
 *  DMTCP:dmtcp/src is free software: you can redistribute it and/or        *
 *  modify it under the terms of the GNU Lesser General Public License as   *
 *  published by the Free Software Foundation, either version 3 of the      *
 *  License, or (at your option) any later version.                         *
 *                                                                          *
 *  DMTCP:dmtcp/src is distributed in the hope that it will be useful,      *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU Lesser General Public License for more details.                     *
 *                                                                          *
 *  You should have received a copy of the GNU Lesser General Public        *
 *  License along with DMTCP:dmtcp/src.  If not, see                        *
 *  <http://www.gnu.org/licenses/>.                                         *
 ****************************************************************************/

#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include "constants.h"
#include "dmtcpalloc.h"
#include "connection.h"
#include "../jalib/jserialize.h"
#include "../jalib/jfilesystem.h"
#include "../jalib/jalloc.h"


namespace dmtcp
{

  class KernelDeviceToConnection;
  class ConnectionToFds;
  class ConnectionState;

  class ConnectionList
  {
      friend class KernelDeviceToConnection;
    public:
#ifdef JALIB_ALLOCATOR
      static void* operator new(size_t nbytes, void* p) { return p; }
      static void* operator new(size_t nbytes) { JALLOC_HELPER_NEW(nbytes); }
      static void  operator delete(void* p) { JALLOC_HELPER_DELETE(p); }
#endif
      typedef dmtcp::map<ConnectionIdentifier, Connection*>::iterator iterator;
      iterator begin() { return _connections.begin(); }
      iterator end() { return _connections.end(); }
      static ConnectionList& instance();
      void erase ( iterator i );
      void erase ( ConnectionIdentifier& key );
      ConnectionList();
      Connection& operator[] ( const ConnectionIdentifier& id );
      Connection *getConnection(const ConnectionIdentifier &id);

      void serialize ( jalib::JBinarySerializer& o );

      //examine /proc/self/fd for unknown connections
      void scanForPreExisting();
    protected:
      void add ( Connection* c );
    private:
      typedef  dmtcp::map<ConnectionIdentifier, Connection*> ConnectionMapT;
      ConnectionMapT _connections;
  };


  class KernelDeviceToConnection
  {
    public:
#ifdef JALIB_ALLOCATOR
      static void* operator new(size_t nbytes, void* p) { return p; }
      static void* operator new(size_t nbytes) { JALLOC_HELPER_NEW(nbytes); }
      static void  operator delete(void* p) { JALLOC_HELPER_DELETE(p); }
#endif
      static KernelDeviceToConnection& instance();
      Connection& retrieve ( int fd );
      Connection *retrieveP ( int fd );
      dmtcp::string getDevice( const ConnectionIdentifier& con );
      void create ( int fd, Connection* c );
      void createPtyDevice ( int fd, dmtcp::string deviceName, Connection* c );

      void erase(const ConnectionIdentifier&);

      dmtcp::string fdToDevice ( int fd , bool noOnDemandConnection = false );

      void dbgSpamFds();

      //fix things up post-restart (all or KernelDevices have changed)
      KernelDeviceToConnection ( const ConnectionToFds& source );


      void serialize ( jalib::JBinarySerializer& o );

      KernelDeviceToConnection();

      void handlePreExistingFd ( int fd );
      void prepareForFork ( );

      //called when a device name changes
      void redirect( int fd, const ConnectionIdentifier& id );
    protected:


    private:
      typedef dmtcp::map< dmtcp::string , ConnectionIdentifier >::iterator iterator;
      dmtcp::map< dmtcp::string , ConnectionIdentifier > _table;
  };

  class ConnectionToFds
  {
    public:
#ifdef JALIB_ALLOCATOR
      static void* operator new(size_t nbytes, void* p) { return p; }
      static void* operator new(size_t nbytes) { JALLOC_HELPER_NEW(nbytes); }
      static void  operator delete(void* p) { JALLOC_HELPER_DELETE(p); }
#endif
      ConnectionToFds() {
      }
      ConnectionToFds ( KernelDeviceToConnection& source );
      dmtcp::vector<int>& operator[] ( const ConnectionIdentifier& c ) { return _table[c]; }

      typedef dmtcp::map< ConnectionIdentifier, dmtcp::vector<int> >::iterator iterator;
      iterator begin() { return _table.begin(); }
      iterator end() { return _table.end(); }
      typedef dmtcp::map< ConnectionIdentifier, dmtcp::vector<int> >::const_iterator const_iterator;
      const_iterator begin() const { return _table.begin(); }
      const_iterator end() const { return _table.end(); }

      size_t size() const { return _table.size(); }
      void erase ( const ConnectionIdentifier& conId );

      void serialize ( jalib::JBinarySerializer& o );

    private:
      dmtcp::map< ConnectionIdentifier, dmtcp::vector<int> > _table;
  };


  // Another mapping from Connection to FD
  // This time to temporarily hold FD's which must be slid around as each FD
  //jis put into use
  class SlidingFdTable
  {
    public:
#ifdef JALIB_ALLOCATOR
      static void* operator new(size_t nbytes, void* p) { return p; }
      static void* operator new(size_t nbytes) { JALLOC_HELPER_NEW(nbytes); }
      static void  operator delete(void* p) { JALLOC_HELPER_DELETE(p); }
#endif
      SlidingFdTable ( int startingFd = 500 )
        : _nextFd ( startingFd )
        , _startFd ( startingFd )
      {}

      int startFd() { return _startFd; }

      ///
      /// retrieve, and if needed assign an FD for id
      int getFdFor ( const ConnectionIdentifier& id );

      ///
      /// if the given FD is in use... reassign it to another FD
      void freeUpFd ( int fd );

      bool isInUse ( int fd ) const;

      static void changeFd ( int oldfd, int newfd );

      void closeAll();
    private:
      dmtcp::map< ConnectionIdentifier, int > _conToFd;
      dmtcp::map< int, ConnectionIdentifier > _fdToCon;
      int _nextFd;
      int _startFd;
  };


  // UniquePtsNameToPtmxConId class holds the UniquePtsName -> Ptmx ConId mapping.
  // This file should not be serialized. The contents are added to this file
  // whenever a /dev/ptmx device is open()ed to create a pseudo-terminal
  // master-slave pair.
  class UniquePtsNameToPtmxConId
  {
    public:
#ifdef JALIB_ALLOCATOR
      static void* operator new(size_t nbytes, void* p) { return p; }
      static void* operator new(size_t nbytes) { JALLOC_HELPER_NEW(nbytes); }
      static void  operator delete(void* p) { JALLOC_HELPER_DELETE(p); }
#endif
      UniquePtsNameToPtmxConId() {}
      static UniquePtsNameToPtmxConId& instance();

      ConnectionIdentifier& operator[] ( dmtcp::string s ) { return _table[s]; }

      dmtcp::Connection& retrieve ( dmtcp::string str );

      dmtcp::string retrieveCurrentPtsDeviceName ( dmtcp::string str );

      typedef dmtcp::map< dmtcp::string, ConnectionIdentifier >::iterator iterator;

      //void serialize ( jalib::JBinarySerializer& o );

      void add ( dmtcp::string str, ConnectionIdentifier cid ) { _table[str] = cid; }

    private:
      dmtcp::map< dmtcp::string, ConnectionIdentifier > _table;
  };

  class MmapManager
  {
    public:
      static MmapManager& instance();
      void handleMmap(void *addr, size_t length, int prot, int flags,
                      int fd, off_t offset);
      void handleMmap64(void *addr, size_t length, int prot, int flags,
                        int fd, off64_t offset);
      void handleMunmap(void *addr, size_t length);
      typedef dmtcp::map< void*, int >::iterator iterator;
    private:
      dmtcp::map< void*, int > _addrToFd;
  };
}

#endif
