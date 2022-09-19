<?php
$uploaddir = './';
$uploadfile = $uploaddir . basename($_FILES['imageupload']['name']);
$imageFileType = strtolower(pathinfo($uploadfile,PATHINFO_EXTENSION));
$uploadOk = 1;

echo "<pre>";
echo $imageFileType;
echo "</pre>";

echo "<pre>";
var_dump($_POST);
echo "</pre>";

if ($imageFileType != 'jpg' && $imageFileType != 'png' && $imageFileType != 'jpeg' && $imageFileType != 'gif')
{
	echo "Your file is not an image! Only .jpg .png .jpeg and .gif are allowed.\n";
	$uploadOk = 0;
}
else
{
	if (move_uploaded_file($_FILES['imageupload']['tmp_name'], $uploadfile))
	{
	    echo "Image is valid, and was successfully uploaded.\n";
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
