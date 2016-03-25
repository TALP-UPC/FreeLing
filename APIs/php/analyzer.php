<?php  

// Auxiliary simplified socket class to connect to analyzer_server.

class FL_socket {

   private $sock;

   /// create a socket and connect to server
   function __construct($host,$port) { 
      $this->sock = socket_create(AF_INET,SOCK_STREAM,0);
      $ok = socket_connect($this->sock,$host,$port);
      if (!$ok) exit(socket_strerror(socket_last_error()));
   }

   // send text to the server
   function write($text) {
      // send text, adding a C end-of-string (i.e. chr(0)) as termination mark
      $i = socket_write($this->sock,$text.chr(0)); 
      if ($i===FALSE) exit(socket_strerror(socket_last_error()));
   }

   // receive message from server
   function read() {

       $BUF_SZ=2048;

       // receive chunks of BUF_SZ, until an C end-of-string
       // (i.e. chr(0)) is detected.

       $b = socket_recv($this->sock, $m, $BUF_SZ, 0);
       if ($b===FALSE) exit(socket_strerror(socket_last_error()));
       $msg = $m;

       while (strpos($m,chr(0))===FALSE) {
         $b = socket_recv($this->sock, $m, $BUF_SZ, 0);
         if ($b===FALSE) exit(socket_strerror(socket_last_error()));
         $msg .= $m;
       }      
       return trim($msg,"\0");
   }   
   
   // tell the server we're not sending anything else.
   function shutdown() {
     socket_shutdown($this->sock,1);
   }
}


/// php API to access FreeLing services, via socket

class analyzer
{
    // name of I/O named pipes
    private $port;
    private $host;
    private $pid;
    
    // -------------------------------------------------------------
    // constructor
    //     parameters:
    //            $host : port --> create new server in local host
    //                    host:port --> connect to existing server in host:port
    //
    //         The following parameters are optional and only relevant when
    //         creating a new server:
    //            $opt  : FreeLing options.  E.g. "-f es.cfg --outf morfo"
    //            $pathIn : path to FreeLing executables 
    //            $pathDat: path to FreeLing data
    //
    // -------------------------------------------------------------

    function __construct($host,$opt="",$pathIn="/usr/local/bin",$pathDat="/usr/local/share/FreeLing") {

    $hp = explode(":",$host);   // split "host:port"

    if (count($hp)==1) {  // only port was given, start a new server there

       $this->host="127.0.0.1";   // localhost
       $this->port=$hp[0];
 
       ## ignore stdout & stderr (we will comunicate via socket).
       $descriptorspec = array(
             0 => array("pipe", "r"),  // stdin is a pipe that the child will read from
             1 => array("file", "/dev/null", "a"),  // stdout is the file the child will write to
             2 => array("file", "/dev/null", "a")   // stderr is a file to write to
           );       
       $pipes= array();  
       ## set environment variables 
       $env = array('FREELINGSHARE' => $pathDat);
       ## launch server
       $cmd = "exec ".$pathIn."/analyze --server --port ".$this->port." ".$opt;

       $this->pid = proc_open($cmd, $descriptorspec, $pipes, NULL, $env);
       ## we won't use stdin/stdout, close our side.
       fclose($pipes[0]);

       sleep(3);  # wait for the server to load
      }
      else if (count($hp)==2) {  // host:port was given, use existing server
       ## using remote server, just remember where it is.
       $this->pid=0;
       $this->host= gethostbyname($hp[0]);
       $this->port= $hp[1];
      }
    else 
      print "Given host:port '$host' is not valid.\n";
    }

    // -------------------------------------------------------------
    // Analyze given text
    // -------------------------------------------------------------
    function analyze_text($text,$outfile="") {

        $output = "";

        // connect to server
        $sk = new FL_socket($this->host,$this->port);

        // sync with server
        $sk->write("RESET_STATS");
        $msg = $sk->read();
        if ($msg!=="FL-SERVER-READY") {
          print "ERROR - server not ready?";
          exit(1);
        }

        // send text
        if (strlen($text)>0) {
           $sk->write($text); 

          // receive server reply    
          $msg = $sk->read();
          // check whether the server is waiting for extra input
          if ($msg!=="FL-SERVER-READY") $output=$msg;
        }

        // tell the server we're not sending anything else.
        $sk->shutdown();

        // receive server reply    
        $msg = $sk->read();
        // check whether the server added something to previous response
        if ($msg!=="FL-SERVER-READY") $output.=$msg;

        // if the output was expected to be written to a file, do so.
        if ($outfile!="") { 
           $of = fopen("$outfile",'w');
           fwrite($of,$output);
           fclose($of);
           $output = "";
        }

     return $output;
    }

    // -------------------------------------------------------------
    //  Analyze given file
    // -------------------------------------------------------------
    function analyze_file($file,$outfile="") {

        $output = "";

        // connect to server
        $sk = new FL_socket($this->host,$this->port);

       $ftx=fopen("$file",'r');
       if (!$ftx) {
         print "Error opening input file '$file'.\n";
         return;
       }
       if ($outfile!="") $of = fopen("$outfile",'w');

       while (!feof($ftx)) {
           $text = fgets($ftx);  // get line

           if (strlen($text)>0) {
              $sk->write($text);   // send text
              $msg = $sk->read();  // receive server reply    
              if ($msg!=="FL-SERVER-READY") {
                // output reply
                if ($outfile!="") fwrite($of,$msg);
                else $output .= $msg;   
             }
           }
        }       

        // tell the server we're not sending anything else.
        $sk->shutdown();

        // receive server reply, in case something remained in the buffer
        $msg = $sk->read();
        if ($msg!=="FL-SERVER-READY") { 
           // output reply
           if ($outfile!="") fwrite($of,$msg);
           else $output .= $msg;   
        }

        fclose($ftx);
        fclose($of);

        return $output;
    }

    // -------------------------------------------------------------
    /// Reset server statistics
    // -------------------------------------------------------------
    function reset_stats(){
        // connect to server
        $sk = new FL_socket($this->host,$this->port);
        // send text
        $sk->write("RESET_STATS"); 
        // receive server reply    
        $msg = $sk->read();
        // check whether the server is waiting for extra input
        if ($msg!=="FL-SERVER-READY") print "Error reseting server stats '$msg'\n";
        // tell the server we're not sending anything else.
        $sk->shutdown();
    }

    // -------------------------------------------------------------
    /// Get server statistics
    // -------------------------------------------------------------
    function print_stats(){
        // connect to server
        $sk = new FL_socket($this->host,$this->port);
        // send text
        $sk->write("PRINT_STATS"); 
        // receive server reply    
        $msg = $sk->read();
        if ($msg!=="FL-SERVER-READY") print $msg;
        // tell the server we're not sending anything else.
        $sk->shutdown();
     }

    // -------------------------------------------------------------
    /// Destructor, kill server
    // -------------------------------------------------------------
    function __destruct() {
      // only kill the process if we created it
      if ($this->pid!=0) {
        proc_terminate($this->pid);
      }
    }
}

?>
