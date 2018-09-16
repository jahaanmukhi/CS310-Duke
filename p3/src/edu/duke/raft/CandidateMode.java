package edu.duke.raft;

import java.util.Timer;
import java.util.Random;

public class CandidateMode extends RaftMode {
    
  private Timer electionTimer;
  private Timer voteTimer;
  
  private int ELECTION_TIMEOUT_ID = 2;
  private int VOTE_TIMEOUT_ID = 3;
  private long VOTE_TIMEOUT_FREQ = 50;
  
  public void go () {
    synchronized (mLock) {      
        //increment current term
        int term = mConfig.getCurrentTerm();
        term++;
        mConfig.setCurrentTerm(term, mID);
        System.out.println ("C: S" + 
          mID + 
          "." + 
          term + 
          ": switched to candidate mode.");
        
        
        //set timer for election
        long timeout_len = getRandom(ELECTION_TIMEOUT_MIN, ELECTION_TIMEOUT_MAX);

        if (mConfig.getTimeoutOverride() > 0) {
          timeout_len = mConfig.getTimeoutOverride();
        }
        electionTimer = scheduleTimer(timeout_len ,ELECTION_TIMEOUT_ID);
        
        //start election
        RaftResponses.setTerm(term);
        RaftResponses.clearVotes(term);

        //checkVoteTimer = scheduleTimer((long) (8, 2));
        
        //request votes from servers
        for(int i = 1; i<=mConfig.getNumServers();i++){
          remoteRequestVote(i, mConfig.getCurrentTerm(), mID, mLog.getLastIndex(), mLog.getLastTerm());
        }
        voteTimer = scheduleTimer(VOTE_TIMEOUT_FREQ, VOTE_TIMEOUT_ID);
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
      int result = term;
      
      if(candidateID==mID){
        System.out.println ("C: S" + mID + "." + term + ": voting for self");
        return 0;
      }
      //candidate term less than server term; reject
      else if(candidateTerm <=term){
        System.out.println ("C: S" + mID + "." + term + ": deny vote from S" + candidateID + "." + candidateTerm);
        return result;
      }
      
      //candidate term greater than server term; set server term, follower mode
      else{
        System.out.println ("C: S" + mID + "." + term + " revert to follower");
        mConfig.setCurrentTerm(candidateTerm, 0);
        electionTimer.cancel();
        voteTimer.cancel();
        RaftServerImpl.setMode(new FollowerMode());
        return result;
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
      int term = mConfig.getCurrentTerm ();
      int result = term;
      //if appendEntries RPC received from new leader: convert to follower
      if(leaderTerm < term){
        return result;
      }
      else if(leaderTerm >= term){
        if (leaderTerm == term) {
          mConfig.setCurrentTerm(leaderTerm, mConfig.getVotedFor());    
        } else {
          mConfig.setCurrentTerm(leaderTerm, 0);  
        }
        electionTimer.cancel();
        voteTimer.cancel();
        RaftServerImpl.setMode(new FollowerMode());
        System.out.println ("C: S" + mID + "." + term + " convert to follower");
        return result;
      } else {
        return result;
      }  
    } 
  }
  
  // @param id of the timer that timed out
  public void handleTimeout (int timerID) {
    synchronized (mLock) {
      int term = mConfig.getCurrentTerm();
      
      //If election timeout elapses, start new election
      
      if(timerID == ELECTION_TIMEOUT_ID){
        electionTimer.cancel();
        voteTimer.cancel();
        go();
    
      } else if(timerID == VOTE_TIMEOUT_ID){
        // voteTimer.cancel();

        int[] voteArr = RaftResponses.getVotes(mConfig.getCurrentTerm());
        int voteCount = 0;
        
        if (voteArr != null) {
            for(int i = 1; i<=mConfig.getNumServers();i++){
              if(voteArr[i] == 0){
                voteCount++;
                if(voteCount > ((mConfig.getNumServers())/2) ) {
                  // voteTimer.cancel();
                  System.out.println ("C: S" + mID + "." + term + " won vote, becomes new leader");
                  electionTimer.cancel();
                  voteTimer.cancel();
                  RaftServerImpl.setMode(new LeaderMode());
                }
              }
              if(voteArr[i] > term){
                electionTimer.cancel();
                // voteTimer.cancel();
                mConfig.setCurrentTerm(voteArr[i],0);
                electionTimer.cancel();
                voteTimer.cancel();
                RaftServerImpl.setMode(new FollowerMode());
                System.out.println ("C: S" + mID + "." + term + " lost vote, back to follower");
              }
            }
        } else {
          // voteTimer.cancel();
          voteTimer = scheduleTimer(VOTE_TIMEOUT_FREQ, VOTE_TIMEOUT_ID);
        }
      }
    }
  }

  public static long getRandom(int low, int high) {
    Random rand = new Random();
    long ret = low + rand.nextInt(Math.abs(high-low));
    return ret; 
  }
}




