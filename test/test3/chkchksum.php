#! /usr/bin/env php
<?php
while($line=fgets(STDIN)) {
    $dsize = intval($line[1].$line[2],16);
    $b = 0;
    for($i=1;$i<strlen($line)-1;$i+=2) {
        $b += intval($line[$i].$line[$i+1],16);
    }
    $b = $b&0xff;
    printf("%s%02X\n",$line,$b);
}
?>
