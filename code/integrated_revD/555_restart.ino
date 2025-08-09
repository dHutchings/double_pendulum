void setup_restart() //this setups the 555-timer based automatic restarting of the pendulum
{
  pinMode(auto_timer_restart,INPUT);
  pinMode(auto_timer_reset,OUTPUT);
  digitalWrite(auto_timer_reset,LOW); //clear the timer.
  delay(1);
  digitalWrite(auto_timer_reset,HIGH); //the normal, "not-pushing state".  Remember that this is opposite of the actual MOSFET command, we're "Inverting" in software.

  attachInterrupt(digitalPinToInterrupt(auto_timer_restart),auto_restart,LOW); //i tried falling, but because TIMER_RESTART is bound to Dio 7 right now, Falling doesn't work and I have to rely on LOW.
  //relying on LOW is bad due to the possibility of chain-calling.
  //I should swich this for one of the three USR pins - which are capable of using falling but don't in favor of a while-true debounce (which I can't use here).
}

void inform_restart_timer(bool MOS_State)
{
  digitalWrite(auto_timer_reset,! MOS_State);
}

void auto_restart()
{
  start_pendulum();
}
