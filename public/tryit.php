<?php include("../php/site/header.php"); ?>

<p><center>
  Go back and forth between JSONSchema and orderly,
  right here in your browser.
  <br />
  Click a box and start editing...
</center></p>

<div class="fullWidth">

<div id="orderly" class="editableCode lhs">
loading...
</div>

<div id="jsonschema" class="editableCode rhs">
loading...
</div>

</div>

<script type="text/javascript" src="/js/jquery-min.js"></script>
<script>
var orderly = "object {\n  string name;\n  string description?;\n  string homepage /^http:/;\n  integer {1500,3000} invented;\n}*;";

var jsonschema = "{\n  \"type\": \"object\",\n  \"properties\": {\n    \"name\": {\n      \"type\": \"string\"\n    },\n    \"description\": {\n      \"type\": \"string\",\n      \"optional\": true\n    },\n    \"homepage\": {\n      \"type\": \"string\",\n      \"pattern\": \"^http:\"\n    },\n    \"invented\": {\n      \"type\": \"integer\",\n      \"minimum\": 1500,\n      \"maximum\": 3000\n    }\n  },\n  \"additionalProperties\": true\n}";

var pendingChange;

function changeIsGood()
{
  var fromType = pendingChange.type;
  var toType = (fromType === "orderly") ? "jsonschema" : "orderly";
  if (fromType === "orderly") { orderly = pendingChange.contents; }
  else { jsonschema = pendingChange.contents; }
  $.post("/api/" + toType, pendingChange.contents, function(data) {
    if (toType === "orderly") {
      orderly = data;
      displayCode("orderly");
    } else {
      jsonschema = data;
      displayCode("jsonschema");
    }
  });
}

function displayCode(who)
{
   if (!who || who === "orderly") {
     $("#orderly").html("<pre class=\"prettyprint\">" + orderly + "</pre>");
     $("#orderly").click(function () {
       displayCode();
       $("#orderly").unbind('click');
       $("#orderly").html("<textarea rows=20>" + orderly + "</textarea>");     
       $("#orderly").focus();
       $("#orderly textarea").keyup(function() {
         if (pendingChange && pendingChange.tid) {
           clearTimeout(pendingChange.tid);
         }
         orderly = $("#orderly textarea").val();
         pendingChange = {
           contents: orderly,
           type: "orderly",
  	 tid: setTimeout(changeIsGood, 1000)
         };
       });
  
       return false;
     });
     if (who) setTimeout(prettyPrint, 1);
   }

   if (!who || who === "jsonschema") {
     $("#jsonschema").html(
         "<pre class=\"prettyprint\">" + jsonschema + "</pre>");
     $("#jsonschema").click(function () {
       displayCode();
       $("#jsonschema").html("<textarea rows=20>" + jsonschema + "</textarea>");
       $("#jsonschema").unbind('click');
       $("#jsonschema").focus();
       $("#jsonschema textarea").keyup(function() {
         if (pendingChange && pendingChange.tid) {
           clearTimeout(pendingChange.tid);
         }
         jsonschema = $("#jsonschema textarea").val();
         pendingChange = {
           contents: jsonschema,
           type: "jsonschema",
           tid: setTimeout(changeIsGood, 1000)
         };
       });
       return false;
     });
     setTimeout(prettyPrint, 1);
    }
}

$(document).ready(function() {
  displayCode();
});

</script>

<?php include("../php/site/footer.php"); ?>
