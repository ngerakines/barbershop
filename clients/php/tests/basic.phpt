--TEST--
simple test
--FILE--
<?php
$bs = new Barbershop();
var_dump($bs);
var_dump($bs->connect('127.0.0.1', 8002));
var_dump($bs->update('5000', 1));
var_dump($bs->peak());
var_dump($bs->score('5000'));
var_dump($bs->next());
var_dump($bs->next());
?>
--EXPECT--
object(Barbershop)#1 (0) {
}
bool(true)
string(3) "+OK"
string(5) "+5000"
string(3) "+1"
string(5) "+5000"
string(3) "+-1"