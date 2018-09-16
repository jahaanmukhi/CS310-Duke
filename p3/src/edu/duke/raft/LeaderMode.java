package edu.duke.raft;

import java.util.Timer;
import java.util.ArrayList;
import java.util.List;

public class LeaderMode extends RaftMode {

  private int mytimerID = 2;
  private Timer mytimer;
  List<Integer> mylist;
  boolean done;
  private int[] responses;
  private int logIdx;

  public void go () {
    synchronized (mLock) {
      int term = mConfig.getCurrentTerm();
      System.out.println ("L: S" + 
        mID + 
        "." + 
        term + 
        ": switched to leader mode.");
      
      // List<Integer> mylist = new ArrayList<Integer>();
      // mylist.add(0);
      // for (int i=1; i<=mConfig.getNumServers(); i++){
      //   mylist.add(mLog.getLastIndex());
      // }
      RaftResponses.setTerm(term);
      RaftResponses.clearAppendResponses(term);
      
      logIdx = mLog.getLastIndex();

      for(int i = 1; i <= mConfig.getNumServers(); i++) {
        if(i != mID) {
          //System.out.println(mID + " sending heartbeat to " + i);
          remoteAppendEntries(i, term, mID, logIdx, mLog.getEntry(logIdx).term, null, mCommitIndex);
        }
      }
      mytimer = scheduleTimer((long)HEARTBEAT_INTERVAL, mytimerID);
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
      int term = mConfig.getCurrentTerm();
      if (term <= candidateTerm) {
        //relinquish leader mode, go back to follower
        //mytimer.cancel();
        System.out.println("L: S" + mID + "." + mConfig.getCurrentTerm() + ": switched to follower mode (relinquish leadermode rV)");
        mConfig.setCurrentTerm(candidateTerm, 0);
        mytimer.cancel();
        RaftServerImpl.setMode(new FollowerMode());
        return 0;
      }
      return term;
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
      int term = mConfig.getCurrentTerm ();
      if (term < leaderTerm) {
        //relinquish leader mode, go back to follower
        mytimer.cancel();
        System.out.println("L: S" + mID + "." + mConfig.getCurrentTerm() + ": switched to follower mode (relinquish leadermode AE)");
        mConfig.setCurrentTerm(leaderTerm, 0);
        RaftServerImpl.setMode(new FollowerMode());
      }
      return term;
    }
  }

  // @param id of the timer that timed out
  public void handleTimeout (int timerID) {
    synchronized(mLock){
      int term = mConfig.getCurrentTerm();
      if (mytimerID == timerID) {
          int[] temp = RaftResponses.getAppendResponses(term);
          responses = temp;
          // responses = RaftResponses.getAppendResponses(term);
      } 
      if (responses != null) {
        repairlog();
      }
      mytimer = scheduleTimer((long) HEARTBEAT_INTERVAL, mytimerID);
    }
  }

  public void repairlog () {
    synchronized (mLock) {
      int term = mConfig.getCurrentTerm();
      int l_lastIdx = mLog.getLastIndex();
      // if (logIdx > 0) {
        //   logIdx --;
        // }
      if (logIdx > 0) {
        logIdx --;
      }
      for (int i = 1; i <= mConfig.getNumServers(); i++) {
        if (i != mID) {
          if (responses[i] == 0) {
            remoteAppendEntries(i, term, mID, logIdx, mLog.getEntry(logIdx).term, null, mCommitIndex);
          } else if (responses[i] <= term) {
            mConfig.setCurrentTerm(responses[i], 0);
            mytimer.cancel();
            RaftServerImpl.setMode(new FollowerMode());
            return;
          } else {
            Entry[] appendArr = new Entry[l_lastIdx - logIdx];
            for (int j = logIdx + 1; j <= l_lastIdx; j++) {
              appendArr[j - l_lastIdx -1] = mLog.getEntry(j);
            }
            remoteAppendEntries(i, term, mID, logIdx, mLog.getEntry(logIdx).term, appendArr, mCommitIndex);
            // int nextIdx = mylist.get(i);
            // Entry[] newEntries = new Entry[mLog.getLastIndex() - nextIdx + 1];
            // for (int j = nextIdx; j <= mLog.getLastIndex(); j++) {
            //     newEntries[j - nextIdx] = mLog.getEntry(j);
            // }
            // int prevIndex = nextIdx - 1;
            // int prevTerm = -1;
            // if (mLog.getEntry(prevIndex) != null) {
            //     prevTerm = mLog.getEntry(prevIndex).term;
            // }
            // remoteAppendEntries(i, mConfig.getCurrentTerm(), mID, prevIndex, prevTerm, newEntries, mCommitIndex);

          }
        // for (int k = 1; k <= mConfig.getNumServers(); k++) {
        //   if (mylist.get(k) > 0) {
        //     mylist.set(k, mylist.get(k)-1);
        //   } else {
        //     mylist.set(k, 0);
        //   }
        // } 
        }
      }


        // responses = RaftResponses.getAppendResponses(term);
        
        // if(responses!=null) {

    //       for (int i = 1; i <= mConfig.getNumServers(); i++) {
    //         while (!done) {
    //             // int logIdx = mylist.get(i);
    //             // if (logIdx > 0) {
    //             //     logIdx --;
    //             // }
    //             if (i != mID) {
    //               //int n = mylist.get(i);
    //              if (responses[i]==0) {
    //                 // remoteAppendEntries(i, term, mID, logIdx, mLog.getEntry(logIdx).term, null, mCommitIndex);
    //               }
    //               else if(responses[i] > term){
    //                 mConfig.setCurrentTerm(responses[i], 0);
    //                 mytimer.cancel();
                    
    //                 System.out.println("L: S" + mID + "." + mConfig.getCurrentTerm() + ": switched to follower mode (relinquish leadermode rL)");
                    
    //                 RaftServerImpl.setMode(new FollowerMode());
    //                 return;
    //               }
                  
    //               else{
    //                 ArrayList<Entry> entryList = new ArrayList<Entry>();
    //                 for(int j = mylist.get(i); j < mLog.getLastIndex()+1; j++){
    //                     entryList.add(mLog.getEntry(i));
    //                 }
    //                 Entry[] entryArr = new Entry[entryList.size()];
    //                 entryArr = entryList.toArray(entryArr);
    //                 remoteAppendEntries(i, term, mID, mylist.get(i), mLog.getEntry(mylist.get(i)).term, entryArr, mCommitIndex);

    //                 int n = mylist.get(i);
    //                 n--;
    //                 mylist.set(i,n);


    //                 // if (s_nextIdx > 0){
    //                 //   s_nextIdx--;
    //                 //   mylist.set(i, s_nextIdx);
    //                 // }
    //                 // Entry[] entries = new Entry[mLog.getLastIndex() - logIdx];
    //                 // for (int j = logIdx; j <= mLog.getLastIndex(); j++) {
    //                 //   entries[j - logIdx] = mLog.getEntry(j);
    //                 // }
                    
    //                 // int s_prevIdx = s_nextIdx - 1;
    //                 // int s_prevTerm = -1;
    //                 // if (mLog.getEntry(s_prevIdx) != null) {
    //                 //   s_prevTerm = mLog.getEntry(s_prevIdx).term;
    //                 // }
    //                 // remoteAppendEntries(i, term, mID, logIdx, mLog.getEntry(logIdx).term, entries, mCommitIndex);
                    
                      
    //               }

    //               //mylist.set(i, mylist.get(i)-1);
    //               //} else {
    //               //mylist.set(i, 0);
    //             }
    //             if (responses[i] == 0) {
    //                 done = true;
    //             }
    //         }
                
        
    //     // }
    //     // mytimer = scheduleTimer((long)HEARTBEAT_INTERVAL, mytimerID);
    //   }
    
    // }
    }
  }
  
  private void heartbeat() {
    //while (true) {
    System.out.println("L: Leader S"+mID + "." + mConfig.getCurrentTerm() +": sending heartbeat");

    for(int i = 1; i <= mConfig.getNumServers(); i++) {
      if(i != mID) {
        //System.out.println(mID + " sending heartbeat to " + i);
        this.remoteAppendEntries(i, mConfig.getCurrentTerm(), mID, logIdx, mLog.getEntry(logIdx).term, null, mCommitIndex);
      }
    }
    // int term = mConfig.getCurrentTerm();
    // logIdx = mLog.getLastIndex();
    // int l_lastIdx = mLog.getLastIndex();
    

    // RaftResponses.setTerm(term);
    // repairlog();
    // RaftResponses.clearAppendResponses(term);

    
    // for (int i=1; i<=mConfig.getNumServers(); i++) {
    //     if(i != mID) {
   //          int logIdx = mylist.get(i);
    //      remoteAppendEntries(i, term, mID, logIdx, mLog.getEntry(logIdx).term, null, mCommitIndex);
    //     }
    // }

    // mytimer = scheduleTimer((long)HEARTBEAT_INTERVAL, mytimerID);

  }

    
}
      
      







