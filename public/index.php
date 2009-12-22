<?php include("../php/site/header.php"); ?>

<div class="narrowContent">
<div id="highlevel">
Orderly is a textual format for describing JSON.
Orderly can be compiled into JSONSchema. 
It is designed to be easy to read and write.
</div>

A little bit of orderly...

<pre class="prettyprint">
object {
  string name;
  string description?;
  string homepage /^http:/;
  integer {1500,3000} invented;
}*;
</pre>

<br>
...describes a little bit of JSON...
<pre class="prettyprint">
{
  "name": "orderly",
  "description": "A schema language for JSON",
  "homepage": "http://orderly-json.org",
  "invented": 2009
}  
</pre>
<br>
...and compiles into JSONSchema.
<pre class="prettyprint">
{
  "type": "object",
  "properties": {
    "name": {
      "type": "string"
    },
    "description": {
      "type": "string",
      "optional": true
    },
    "homepage": {
      "type": "string",
      "pattern": "^http:"
    },
    "invented": {
      "type": "integer",
      "minimum": 1500,
      "maximum": 3000
    }
  },
  "additionalProperties": true
}
</pre>
<br><br>
</div>

<?php include("../php/site/footer.php"); ?>
