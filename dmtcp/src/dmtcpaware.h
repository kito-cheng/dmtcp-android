/***************************************************************************
 *   Copyright (C) 2008 by Jason Ansel                                     *
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

#ifndef DMTCPAWARE_H
#define DMTCPAWARE_H

#ifdef __cplusplus
extern "C" {
#endif

#define DMTCP_AFTER_CHECKPOINT 1
#define DMTCP_AFTER_RESTART    2 

// pointer to a "void X();" function
typedef void (*DmtcpFunctionPointer)(void);

/// returned by dmtcpGetCoordinatorStatus()
typedef struct _DmtcpCoordinatorStatus {

  // number of processes connected to dmtcp_coordinator
  int numProcesses;

  // 1 if all processes connected to dmtcp_coordinator are in a running state
  int isRunning;

} DmtcpCoordinatorStatus;

/// returned by dmtcpGetLocalStatus()
typedef struct _DmtcpLocalStatus {

  // the number of times this process has been checkpointed (excludes restarts)
  int numCheckpoints;

  // the number of times this process has been restarted
  int numRestarts;

  // filename of (large) .mtcp checkpoint file (memory/threads) for this process
  const char* checkpointFilenameMtcp; 

  // filename of (tiny) .dmtcp checkpoint file (connection table) for this process
  const char* checkpointFilenameDmtcp;

  // the DMTCP cluster-wide unique process identifier for this process
  // format is "HostHash-PID-Timestamp"
  const char* uniquePidStr;

} DmtcpLocalStatus;


/**
 * Returns 1 if executing under dmtcp_checkpoint, 0 otherwise
 */
int dmtcpIsEnabled();

/**
 * Checkpoint the entire distributed computation, block until checkpoint is
 * complete.
 * - returns DMTCP_AFTER_CHECKPOINT if the checkpoint succeeded.
 * - returns DMTCP_AFTER_RESTART    after a restart.
 * - returns <=0 on error.
 * Should only be called if dmtcpIsEnabled()==1
 */
int dmtcpCheckpointBlocking();

//aliases for ease of use
#define dmtcpCheckpointNonblocking() dmtcpRunCommand('c') 

/**
 * Send a command to the dmtcp_coordinator as if it were typed on the console.
 * Return 1 if command was sent and well-formed, <= 0 otherwise.
 * Should only be called if dmtcpIsEnabled()==1
 */
int dmtcpRunCommand(char command);

/**
 * Gets the coordinator-specific status of DMTCP.
 * Calling this function invalidates older DmtcpCoordinatorStatus structures.
 * Should only be called if dmtcpIsEnabled()==1
 */
const DmtcpCoordinatorStatus* dmtcpGetCoordinatorStatus();

/**
 * Gets the local-node-specific status of DMTCP.
 * Calling this function invalidates older DmtcpLocalStatus structures.
 * Should only be called if dmtcpIsEnabled()==1
 */
const DmtcpLocalStatus* dmtcpGetLocalStatus();

/**
 * Sets the hook functions that DMTCP calls when it checkpoints/restarts. 
 * - These functions are called from the DMTCP thread while all user threads
 *   are suspended.
 * - First preCheckpoint() is called, then either postCheckpoint() or
 *   postRestart() is called.
 * - Set to NULL to disable.
 * Should only be called if dmtcpIsEnabled()==1
 */
void dmtcpInstallHooks( DmtcpFunctionPointer preCheckpoint
                      , DmtcpFunctionPointer postCheckpoint
                      , DmtcpFunctionPointer postRestart);


#ifdef __cplusplus
} //extern "C"
#endif

#endif //DMTCPAWARE_H