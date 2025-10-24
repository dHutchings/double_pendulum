void setup_restart() //this setups the 555-timer based automatic restarting of the pendulum
{
  pinMode(auto_restart,INPUT);  //timer restart is the timer telling us that it's time to restart the pendulum
  pinMode(timer_rst,OUTPUT);  //timer reset resets the timer
  digitalWrite(timer_rst,LOW); //clear the timer.
  delay(1);
  digitalWrite(timer_rst,HIGH); //the normal, "not-pushing state", per 555_test.  remember it used to be inverted.

  attachInterrupt(digitalPinToInterrupt(auto_timer_restart),auto_restart,LOW); //i tried falling, but because TIMER_RESTART is bound to Dio 7 right now, Falling doesn't work and I have to rely on LOW.
  //relying on LOW is bad due to the possibility of chain-calling.
  //I should swich this for one of the three USR pins - which are capable of using falling but don't in favor of a while-true debounce (which I can't use here).
}

void inform_restart_timer(bool MOS_State)
{
  digitalWrite(timer_rst,!MOS_State);
}

void auto_restart()
{
  //detach the interrupt as soon as possible to prevent chain-calling.
  detachInterrupt(digitalPinToInterrupt(auto_timer_restart)); //i tried falling, but because TIMER_RESTART is bound to Dio 7 right now, Falling doesn't work and I have to rely on LOW.

  REASON_FOR_POWERUP = AUTO_RESTART;
}
