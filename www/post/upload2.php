<?php
$uploaddir = './';
$uploadfile = $uploaddir . basename($_FILES['file']['name']);
$fileType = strtolower(pathinfo($uploadfile,PATHINFO_EXTENSION));
$uploadOk = 1;

echo "<pre>";
echo $fileType;
echo "</pre>";

echo "<pre>";
var_dump($_POST);
echo "</pre>";

if ($fileType != 'cpp' && $fileType != 'hpp' && $fileType != 'txt')
{
	echo "Your file is invalid! Only .cpp .hpp and .txt are allowed.\n";
	$uploadOk = 0;
}
else
{
	if (move_uploaded_file($_FILES['file']['tmp_name'], $uploadfile))
	{
	    echo "File is valid, and was successfully uploaded.\n";
		$uploadOk = 1;
	}
	else
	{
		echo "Invalid upload!\n";
		$uploadOk = 0;
	}
}

#phpinfo();

?>
