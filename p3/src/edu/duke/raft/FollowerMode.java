package edu.duke.raft;

import java.util.Timer;
import java.util.Random;


public class FollowerMode extends RaftMode {
  private Timer mytimer;
  private int mytimerID = 1;

  public void go () {
    synchronized (mLock) {
      int term = mConfig.getCurrentTerm();
      System.out.println ("F: S" + 
        mID + 
        "." + 
        term + 
        ": switched to follower mode.");
      setTimer();
    }
  }
  
  // @param candidate’s term
  // @param candidate requesting vote
  // @param index of candidate’s last log entry
  // @param term of candidate’s last log entry
  // @return 0, if server votes for candidate; otherwise, server's
  // current term
  public int requestVote (int candidateTerm,
        int candidateID,
        int lastLogIndex,
        int lastLogTerm) {
    synchronized (mLock) {
      setTimer();
      int term = mConfig.getCurrentTerm ();
      int votedFor = mConfig.getVotedFor();
      int logTerm = mLog.getLastTerm();
      int logIdx = mLog.getLastIndex();
      if ( (term <= candidateTerm) && (votedFor == 0) && (logTerm <=  lastLogTerm) && (logIdx <= lastLogIndex)   )  {
        //vote for candidate
        System.out.println("F: S" + mID + "." + term +
                        ": voted for S" + candidateID + ".");
        mConfig.setCurrentTerm(candidateTerm, candidateID);
        return 0;
      } else {
        //don't vote for candidate
        mConfig.setCurrentTerm(candidateTerm, 0);
        return term;
      }

      
    }
  }
  

  // @param leader’s term
  // @param current leader
  // @param index of log entry before entries to append
  // @param term of log entry before entries to append
  // @param entries to append (in order of 0 to append.length-1)
  // @param index of highest committed entry
  // @return 0, if server appended entries; otherwise, server's
  // current term
  public int appendEntries (int leaderTerm,
          int leaderID,
          int prevLogIndex,
          int prevLogTerm,
          Entry[] entries,
          int leaderCommit) {
    synchronized (mLock) {
      // System.out.println("S" + mID + "." + term + ": append request received from S" + leaderID + "." + leaderTerm + ", pLT=" + prevLogTerm + " pLI=" + prevLogIndex);
      setTimer();
      int term = mConfig.getCurrentTerm();
      
      if (term <= leaderTerm) {
        mConfig.setCurrentTerm(leaderTerm, 0);
      }

      if((mLog.getEntry(prevLogIndex)==null) || (mLog.getEntry(prevLogIndex).term !=prevLogTerm)){
         return -1;
      }

      mLog.insert(entries, prevLogIndex, prevLogTerm);
      System.out.println ("F: S" + mID + "." + term + " insert to log");
      if(leaderCommit > mCommitIndex){
          mCommitIndex = leaderCommit;
      }

      //int index = mLog.insert(entries, prevLogIndex, prevLogTerm);
      
      //if (index == -1) {
      //  return -1;
      //}
      return 0;
    }
  }  

  // @param id of the timer that timed out
  public void handleTimeout (int timerID) {
    synchronized (mLock) {
      if (timerID == mytimerID) {
        mytimer.cancel();
        RaftServerImpl.setMode(new CandidateMode());
        System.out.println("F: S" + mID + " switch to candidate mode");
      }
    }
  }

  public static long getRandom(int low, int high) {
    Random rand = new Random();
    long ret = low + rand.nextInt(Math.abs(high-low));
    return ret; 
  }

  private void setTimer() {
    if (mytimer != null) {
      mytimer.cancel();
      System.out.println("F: S" + mID + " detected time out from leader");
    } 
    long timeout_len = getRandom(ELECTION_TIMEOUT_MIN, ELECTION_TIMEOUT_MAX);
    if (mConfig.getTimeoutOverride() > 0) {
      timeout_len = mConfig.getTimeoutOverride();
    }
    mytimer = scheduleTimer(timeout_len, mytimerID);
  }
}

