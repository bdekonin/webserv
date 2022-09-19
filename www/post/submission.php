<?php
if (!empty($_GET))
	echo $_GET["name"] . " picked " . $_GET["game"] . " as their favourite game with GET method.";
else if (!empty($_POST))
	echo $_POST["name"] . " picked " . $_POST["game"] . " as their favourite game with POST method.";

phpinfo();
?>
