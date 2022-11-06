<?php
if (!empty($_GET))
	echo $_GET["name"] . " has message " . $_GET["message"];
else if (!empty($_POST))
	echo $_POST["name"] . " has message " . $_POST["message"];
// phpinfo();
print_r($_POST);
?>
