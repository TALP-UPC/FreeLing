<? 

 include("analyzer.php");

 //// -----------------------------------------------------
 //// you can connect to an existing server on any machine.
 ////  Note that this will only work if a server has been previously 
 ////  launched on that machine:port, either from a php program, or
 ////  from the command line, using the "analyze" script.
 //
 $b = new analyzer("myserver.home.org:12345");
 // analyze text
 $output = $b->analyze_text("los niños comen pescado");
 print $output;

?>
