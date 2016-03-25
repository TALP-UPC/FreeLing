<? 

 include("analyzer.php");

 // Adjust this path to your local FreeLing installation
 $FL_DIR = "/usr/local";

 // launch a server at port 12345
 $a = new analyzer("12345","-f $FL_DIR/share/freeling/config/es.cfg","$FL_DIR/bin","$FL_DIR/share/freeling");

 // or connect to an existing server to given host:port
 //  $a = new analyzer("localhost:12345");

 // analyze a text
 $output = $a->analyze_text("los niños comen pescado");
 print $output;

 // You can also analyze a text file (and send results to another file in this example)
 // $output = $a->analyze_file("myfile.txt","myfile.out");

?>
