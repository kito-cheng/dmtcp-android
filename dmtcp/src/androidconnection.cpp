/****************************************************************************
 *   Copyright (C) 2006-2010 by Jason Ansel, Kapil Arya, and Gene Cooperman *
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

// THESE INCLUDES ARE IN RANDOM ORDER.  LET'S CLEAN IT UP AFTER RELEASE. - Gene
#include "constants.h"
#include "syscallwrappers.h"
#include "connection.h"
#include  "../jalib/jassert.h"
#include  "../jalib/jfilesystem.h"
#include  "../jalib/jconvert.h"
#include "kernelbufferdrainer.h"
#include "syscallwrappers.h"
#include "connectionrewirer.h"
#include "connectionmanager.h"
#include "dmtcpmessagetypes.h"
#include "util.h"
#include "resource_manager.h"
#include  "../jalib/jsocket.h"
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <termios.h>
#include <iostream>
#include <ios>
#include <fstream>
#include <linux/limits.h>
#include <arpa/inet.h>
#include <linux/ashmem.h>
#include <linux/binder.h>

////////////
///// SPECIAL DEV CHECKPOINTING

void dmtcp::LoggerConnection::preCheckpoint ( const dmtcp::vector<int>& fds,
                                              KernelBufferDrainer& drain ){
}

void dmtcp::LoggerConnection::postCheckpoint ( const dmtcp::vector<int>& fds,
                                               bool isRestart ) {
  if (isRestart)
    restoreOptions ( fds );
}

dmtcp::LoggerConnection::LoggerType
dmtcp::LoggerConnection::path2logger(const char *path) {
  if (strcmp("/dev/log/events", path) == 0) return EVENT_LOGGER;
  if (strcmp("/dev/log/main", path) == 0)   return MAIN_LOGGER;
  if (strcmp("/dev/log/radio", path) == 0)  return RADIO_LOGGER;
  if (strcmp("/dev/log/system", path) == 0) return SYSTEM_LOGGER;
  JASSERT(false) (path).Text("Unknow Logger type");
  return UNKNOWN_LOGGER;
}

const char *dmtcp::LoggerConnection::logger2path(LoggerType type) {
  switch (type) {
    case EVENT_LOGGER:  return "/dev/log/events";
    case MAIN_LOGGER:   return "/dev/log/main";
    case RADIO_LOGGER:  return "/dev/log/radio";
    case SYSTEM_LOGGER: return "/dev/log/system";
    default:
      JASSERT(false) (type).Text("Unkown logger type");
      return NULL;
  }
}

void dmtcp::LoggerConnection::restore(const dmtcp::vector<int>& fds,
                                      dmtcp::ConnectionRewirer*) {
  JTRACE("Restore logger connection") (fds);
  int tmpFd = -1;
  bool needToClose = true;
  const char *path = logger2path((LoggerType)_logger_type);
  tmpFd = _real_open (path, _fcntlFlags);
  for(size_t i=0; i<fds.size(); ++i){
    int fd = fds[i];
    if (fd == tmpFd) needToClose = false;
    errno = 0;
    int ret = _real_dup2 ( tmpFd, fd );
    JASSERT ( ret == fd ) ( tmpFd ) ( fd ) ( JASSERT_ERRNO );
  }
  if (needToClose)
    _real_close (tmpFd);
}

void dmtcp::LoggerConnection::restoreOptions ( const dmtcp::vector<int>& fds ){
}

void dmtcp::LoggerConnection::serializeSubClass ( jalib::JBinarySerializer& o ){
  JSERIALIZE_ASSERT_POINT ( "dmtcp::LoggerConnection" );
  o & _logger_type;
}

void dmtcp::LoggerConnection::mergeWith ( const Connection& that ){
}

////////////
///// ASHMEM DEV CHECKPOINTING

void dmtcp::AshmemConnection::preCheckpoint ( const dmtcp::vector<int>& fds,
                                              KernelBufferDrainer& drain ){
  JTRACE ("Checkpointing ashmem") (fds[0]) (id()) (_addr) (_size);
  if (_addr) {
    std::copy((char *)_addr, (char *)_addr+_size, _data.begin());
    JTRACE("data") (_data);
    struct ashmem_pin pin = { 0, 0 };
    _real_ioctl(fds[0], ASHMEM_UNPIN, &pin);
    _real_munmap(_addr, _mmap_len);
  }
}

void dmtcp::AshmemConnection::postCheckpoint ( const dmtcp::vector<int>& fds,
                                               bool isRestart ) {
  if (isRestart)
    restoreOptions ( fds );
  JTRACE ("Restoring ashmem content") (fds[0]) (id()) (_addr) (_size);
  //nothing
  JASSERT( fds.size() == 1 );
  if (_addr) {
    KernelDeviceToConnection::instance().dbgSpamFds();
    void * _new_addr = _real_mmap(_addr, _mmap_len, _mmap_prot,
                                  _mmap_flags, fds[0], _mmap_off);
    void *_final_addr = _new_addr;
    if (_new_addr != _addr) {
      _final_addr = (void*)_real_syscall(__NR_mremap, _new_addr, _mmap_len,
                                         _mmap_len, MREMAP_FIXED | MREMAP_MAYMOVE,
                                         _addr);
    }
    JASSERT (_final_addr == _addr) (_new_addr) (_addr) (_final_addr)
            (_mmap_len) (_mmap_prot) (_mmap_flags) (_mmap_off) (JASSERT_ERRNO);
    std::copy(_data.begin(), _data.end(), (char *)_addr);
    struct ashmem_pin pin = { 0, 0 };
    if (_pinned) {
      _real_ioctl(fds[0], ASHMEM_PIN, &pin);
    } else {
      _real_ioctl(fds[0], ASHMEM_UNPIN, &pin);
    }
  }
}
void dmtcp::AshmemConnection::restore ( const dmtcp::vector<int>& fds,
                                        ConnectionRewirer* ){
  // nothing
}

void dmtcp::AshmemConnection::restoreOptions ( const dmtcp::vector<int>& fds ){
  JTRACE ("Restoring ashmem fd") (fds[0]) (id()) (_addr) (_size);
  for(size_t i=0; i<fds.size(); ++i){
    int fd = fds[i];
    int tmpFd = -1;
    tmpFd= _real_open("/dev/ashmem", O_RDWR);
    char buf[ASHMEM_NAME_LEN];
    strlcpy(buf, _name.c_str(), sizeof(buf));
    _real_ioctl ( tmpFd, ASHMEM_SET_NAME, buf );

    size_t size = _size;
    _real_ioctl ( tmpFd, ASHMEM_SET_SIZE, size );
    errno = 0;
    int ret = _real_dup2 ( tmpFd, fd );
    JASSERT ( ret == fd ) ( tmpFd ) ( fd ) ( JASSERT_ERRNO );
    if (tmpFd != fd)
      _real_close (tmpFd);
  }
  KernelDeviceToConnection::instance().dbgSpamFds();
}

void dmtcp::AshmemConnection::serializeSubClass ( jalib::JBinarySerializer& o ){
  JSERIALIZE_ASSERT_POINT ( "dmtcp::AshmemConnection" );
  //JTRACE("Serializing STDIO") (id());
  o & _name & _size & _addr;
  o & _mmap_len & _mmap_prot;
  o & _mmap_flags & _mmap_off;
  o & _data & _pinned;
}

void dmtcp::AshmemConnection::mergeWith ( const Connection& that ){
  //Connection::mergeWith(that);
}

void dmtcp::AshmemConnection::restartDup2(int oldFd, int newFd) {
  static ConnectionRewirer ignored;
  restore(dmtcp::vector<int>(1,newFd), &ignored);
}

int dmtcp::AshmemConnection::ioctl(int fd, int request, va_list args) {
  JTRACE ("Handle ioctl for ashmem") ( id() ) ( request );
  va_list local_ap;
  va_copy(local_ap, args);

  if (request == ASHMEM_SET_NAME) {
    char *_new_name = va_arg(local_ap, char*);
    if (_new_name) {
      _name = _new_name;
    }
    JTRACE ("set name for ashmem") ( id() ) (_name);
  } else if (request == ASHMEM_SET_SIZE) {
    size_t _new_size = va_arg(local_ap, size_t);
    _size = _new_size;
    _data.resize(_size);
    JTRACE ("set size for ashmem") ( id() ) (_size);
  } else if (request == ASHMEM_SET_PROT_MASK) {
    _mmap_prot = va_arg(local_ap, int);
  } else if (request == ASHMEM_PIN) {
    _pinned = true;
    JTRACE ("set pinned for ashmem") ( id() ) (_size);
  } else if (request == ASHMEM_UNPIN) {
    _pinned = false;
    JTRACE ("set unpinned for ashmem") ( id() ) (_size);
  } else {
    JTRACE ("Unhandle ioctl for ashmem!") ( id() ) ( request );
  }
  va_end(local_ap);
  return Connection::ioctl(fd, request, args);
}

void dmtcp::AshmemConnection::mmap(void *addr, size_t len, int prot,
                                   int flags, int fd, off_t off) {
  JTRACE ("handle mmap for ashmem") ( id() ) ( addr )
         ( len ) (prot) ( flags ) ( off );
  _addr = addr;
  _mmap_len = len;
  _mmap_prot = prot;
  _mmap_flags = flags;
  _mmap_off = off;
}

void dmtcp::AshmemConnection::mmap64(void *addr, size_t len, int prot,
                                     int flags, int fd, off64_t off) {
  JTRACE ("handle mmap64 for ashmem") ( id() ) ( addr )
         ( len ) (prot) ( flags ) ( off );
  _addr = addr;
  _mmap_len = len;
  _mmap_prot = prot;
  _mmap_flags = flags;
  _mmap_off = off;
}

void dmtcp::AshmemConnection::munmap(void *addr, size_t len) {
  JTRACE ("handle munmap for ashmem") ( id() ) ( addr ) ( len );
  JASSERT( addr == _addr );
  JASSERT( len == _mmap_len );
  _addr = 0;
  _mmap_len = 0;
  _mmap_prot = 0;
  _mmap_flags = 0;
  _mmap_off = 0;
}

////////////
///// PROPERTY CHECKPOINTING

static bool findProp(dmtcp::Util::ProcMapsArea *area){
  int fd = _real_open ( "/proc/self/maps", O_RDONLY);
  dmtcp::string path;

  if( fd < 0 ){
    JTRACE("Cannot open /proc/self/maps file");
    return false;
  }

  while( dmtcp::Util::readProcMapsLine(fd, area) ){
    path = area->name;
    JNOTE("Inspect new /proc/seft/maps line")(path);
    if( path.size() == 0 ){
      JNOTE("anonymous region, skip");
      continue;
    }

    if( path.find("/dev/__properties__") != dmtcp::string::npos ){
      _real_close(fd);
      return true;
    }
  }

  _real_close(fd);
  JASSERT(false).Text("Not found property region???");
  return false;
}

void dmtcp::PropertyConnection::preCheckpoint ( const dmtcp::vector<int>& fds,
                                                KernelBufferDrainer& drain ){
  JTRACE ("Checkpointing property") (fds[0]);
  /* Scan for property map region */
  dmtcp::Util::ProcMapsArea area;
  bool ret = findProp(&area);
  if (ret) {
    _addr = area.addr;
    _size = area.size;
  }
  if (_addr) {
    _real_munmap(_addr, _size);
  }
}

void dmtcp::PropertyConnection::postCheckpoint ( const dmtcp::vector<int>& fds,
                                                 bool isRestart ) {
  if (isRestart)
    restoreOptions ( fds );
  JTRACE ("Restoring property content") (fds[0]) (_addr);
  //nothing
  JASSERT( fds.size() == 1 );
  if (_addr) {
    KernelDeviceToConnection::instance().dbgSpamFds();
    /*
    int fd;
    unsigned sz;
    char *env = getenv("ANDROID_PROPERTY_WORKSPACE");
    JASSERT(env).Text("Can't get ANDROID_PROPERTY_WORKSPACE");
    fd = atoi(env);
    env = strchr(env, ',');
    sz = atoi(env + 1);
    */
    void *_new_addr = _real_mmap(_addr, _size,
                               PROT_READ, MAP_SHARED, fds[0], 0);
    //void * _new_addr = _real_mmap(_addr, _mmap_len, _mmap_prot,
    //                              _mmap_flags, fds[0], _mmap_off);
    JTRACE("Trace property") (_new_addr) (_addr);
    JASSERT(_new_addr == _addr) (_new_addr) (_addr);
  }
}
void dmtcp::PropertyConnection::restore ( const dmtcp::vector<int>& fds,
                                          ConnectionRewirer* ){
  // nothing
}

void dmtcp::PropertyConnection::restoreOptions ( const dmtcp::vector<int>& fds ){
  JTRACE ("Restoring property fd") (fds[0]);
  JASSERT(fds.size() == 1);
  KernelDeviceToConnection::instance().dbgSpamFds();
}

void dmtcp::PropertyConnection::serializeSubClass ( jalib::JBinarySerializer& o ){
  JSERIALIZE_ASSERT_POINT ( "dmtcp::PropertyConnection" );
  o & _size & _addr;
}

void dmtcp::PropertyConnection::mergeWith ( const Connection& that ){
}

void dmtcp::PropertyConnection::restartDup2(int oldFd, int newFd) {
  restore(dmtcp::vector<int>(1,newFd), NULL);
}
