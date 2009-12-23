<?php
header("Content-type: text/plain");

$cmd = "/home/orderly-json.org/bin/orderly_reformat -i jsonschema -o orderly 2>&1";
$in = file_get_contents('php://input');

$spec = array(array("pipe", "r"), array("pipe", "w"), array("pipe", "w")); 

$process = proc_open($cmd, $spec, $pipes, null, $_ENV); 
if (is_resource($process)) {
    # send command
    fwrite($pipes[0], $in);
    fclose($pipes[0]);

    # read pipe for result
    while ($r = fread($pipes[1],1024)) { echo $r; }

    # close pipes
    fclose($pipes[1]);
    $return_value = proc_close($process);
}
?>