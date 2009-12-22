<?php
include("../php/site/header.php");
include("../php/lib/markdown/markdown.php");
print Markdown(file_get_contents("../data/docs.md"));
include("../php/site/footer.php");
?>
