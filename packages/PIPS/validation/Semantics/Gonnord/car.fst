//Generated by c2fsm -s 
model car {
var d,s,t;
//@parameters;
states start,lbl_13,lbl_14,lbl_5,lbl_7,lbl_8;
transition t_34 :={
  from  := start;
  to    := lbl_5;
  guard := true;
  action:= d' = 0, s' = 0, t' = 0;
};
transition t_22 :={
  from  := lbl_13;
  to    := lbl_14;
  guard := true;
  action:=;
};
transition t_23 :={
  from  := lbl_13;
  to    := lbl_5;
  guard := true;
  action:=;
};
transition fp_3 :={
  from  := lbl_14;
  to    := lbl_5;
  guard := ( (9 <= d) || (2 <= s) );
  action:=;
};
transition t_26 :={
  from  := lbl_14;
  to    := lbl_14;
  guard := ( (d <= 8) && (s <= 1) );
  action:= d' = d+1, s' = s+1;
};
transition t_21 :={
  from  := lbl_5;
  to    := lbl_7;
  guard := true;
  action:=;
};
transition t_27 :={
  from  := lbl_7;
  to    := lbl_8;
  guard := true;
  action:=;
};
transition t_28 :={
  from  := lbl_7;
  to    := lbl_13;
  guard := true;
  action:=;
};
transition fp_6 :={
  from  := lbl_8;
  to    := lbl_13;
  guard := (3 <= t);
  action:=;
};
transition t_31 :={
  from  := lbl_8;
  to    := lbl_8;
  guard := (t <= 2);
  action:= s' = 0, t' = t+1;
};
}
strategy dumb {
    Region init := { state = start && true };
}

