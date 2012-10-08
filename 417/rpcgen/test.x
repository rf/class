program GETNAME {
  version GET_VERS {
    long GET_ID(string<50>) = 1;
    string GET_ADDR(long) = 2;
  } = 1;
} = 0x31223456;
