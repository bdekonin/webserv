<?php
if (!empty($_GET))
	echo $_GET["name"] . " has " . $_GET["email"] . " and message " . $_GET["message"];
else if (!empty($_POST))
	echo $_POST["name"] . " has " . $_POST["email"] . " and message " . $_POST["message"];

phpinfo();
?>
