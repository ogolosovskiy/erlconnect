# erlconnect  
erlang c-node library  


$ ./cnodetest     
 client init: node1@192.168.1.80 secretcookie    
 Node  node777@192.168.1.80 10   
 Connecting to node1@192.168.1.80    
 Connected to node1@192.168.1.80    
 loop run   
  run test    
 node node777@192.168.1.80: erl_rpc    
 node node777@192.168.1.80: done 0   
  join thread    
 incoming message:rex ok    
   
 fun:foo    
 args    
 keep alive packet received    
 node node777@192.168.1.80: erl_receive_msg error or node down    
   
  
   
 $ erl -compile ./testserver.erl   
 $ erl -name node1@192.168.1.80 -setcookie secretcookie  
 Erlang/OTP 18 [erts-7.3] [source] [64-bit] [smp:8:8] [async-threads:10] [hipe] [kernel-poll:false]  
   
 Eshell V7.3  (abort with ^G)  
 (node1@192.168.1.80)1> testserver.beam     testserver.erl        
   
 (node1@192.168.1.80)1> testserver:call_cnode({foo, args}).  
 ok  
 (node1@192.168.1.80)2>   
 User switch command  
  --> q  
  
 