Abstract
--------

Orderly is an ergonomic micro-language that can represent a subset of [JSONSchema](http://json-schema.org).  Orderly is designed to _feel_ familiar to the average programmer and to be extremely easy to learn and remember.  This document provides a conversational overview of orderly as well as a normative grammar.

A subset of JSONSchema
----------------------

JSONSchema attempts to provide a representation for three distinct types of information about JSON structures:

* **Data structure** (for documentation and validation purposes)
* **Storage attributes** (information pertinent to tools that wish to persist JSON data) 
* **Interaction Control** (providing hints on how to render a UI where data can be manipulated).

Orderly purposefuly ignores all features of JSONSchema which aren't useful for validation, including the following attributes:
 
* options (label/value)
* title
* description
* transient
* hidden
* disallow
* extends
* identity

An exhaustive list of the differences between orderly and JSONSchema is below.

## A Non-normative Tutorial

A collection of Non-normative examples of Orderly:

### comments and whitespace

Orderly supports comments, comments are initiated with either '#' or '//' and continue to the first encountered newline ('\n').

Orderly doesn't rely overmuch on whitespace, leaving the decision of how to format your schema up to you.   

### property names

Property names may be anything that is allowed inside [JSON strings](http://json.org/).  Unlike JSON itself, however, orderly provides a shorthand where a subset of strings may be represented without quotes.  For instance these are all valid orderly:

    string foo;
    string "foo";
    string "this is a property name with spaces"; 

### common properties

From the JSONSchema specification, four options exist which apply to all data types:

The `optional` property indicates a value which is not required in a conformant JSON instance.  Optional values are represented in orderly with a trailing question mark:

    string name?;
    string "name"?;


The `requires` property indicates a that if a value is present in the instance JSON, another named value must also be present.  In orderly a requirement on another type is expressed by placing the property name (optionally quoted) enclosed in angle brackets at the end of a type definition:

    string town <state>;

Multiple properties may required, and should be separated with commas:

    string town <state,zip>;
    
The `enum` propery specifies a set of allowable values for a key in the json document.

    string mood [ "happy", "sad", "meh" ];
    integer secretOfLife [ 7, 42 ];

In a JSONSchema document the `default` property specifies a default value for a property.  One could imagine that as an input object passes validation it will be automatically augmented with default values for required properties missing in the instance object.  The specification of default values in orderly looks something like assignment in most programming languages:

    string mood [ "happy", "sad", "meh" ] = "happy"; # optimistically default to "happy"

### string types

Strings are specified in orderly using the `string` type specifier.  Strings in JSONSchema support "minLength" and "maxLength" properties, which are represented in orderly using curly braces immediately after the type:  

    string{4,12} login;

Omission of a specification of either minimum or maximum is allowed:

    string{4,} login; # login requires at least 4 chars
    string{,32} name; # name may not be longer than 32 chars

Regular expressions are supported in JSONSchema for string values.  In orderly you may directly provide a regular expression using '/' syntax to denote the beginning and end of the regular expression:

    string mood /^((happy)|(sad)|(meh))$/;

### number & integer types

Numbers are specified in orderly using the `number` type specifier.  In JSONSchema numbers and integers support ranges, in orderly these ranges for numbers are specified in the same way we specify ranges for strings:

    number{0.02, 0.98} numNum;
    integer{0,10} rating;

Syntactically, numbers in orderly follow the same rules as numbers in JSON.

### boolean types

Boolean types are represented in orderly using the `boolean` type specifier:

    boolean iShouldStay;

### object types

Objects are represented in orderly using the `object` type specifier:

    object {
        string foo;
        integer bar;
        number baz;
    };

Object definitions may be "closed", meaning that properties that are not explicitly mentioned are not allowed, or "open".  A trailing star (`*`) indicates an "open" object defintion:

    object {
        string foo;
        # whatever other properties you want, thanks to that star
    }*;


### array types

Arrays are specified using the `array` type specifier.  Schemas for arrays elements may be specified in one of two ways.  First we can specify a single schema that governs all array members, with the schema enclosed by square brackets:

    array [
        numbers{0.00, 1.00};
    ] weights; # an array of floating point weights between 0 and 1.

Alternately, "tuple typing" may be used to specify the allowable values for an array, in this case a list of schemas that apply to each member of the array in sequence:

    array {
        integer;
        string;
        number;
    } artificial;

Finally, when tuple typing is used, the `*` operator may be used to allow additional elements at the end of an array.  For instance, to specify an array where
 the first element is an integer and the remaining are of arbitrary number and type, one might use the following schema:

    array { integer; }* intFollowedByWhatever; 

Finally, array types also support range semantics, for min/max number of elements:

    array { integer; } {0,10} myArrayOfSmallInts;

### handling "additional properties" in arrays and objects

JSONSchema provides the `additionalProperties` attribute which allows a schema author to either:

* specify that a valid instance object/array may not have any properties not in the schema 
* specify an additional schema that applies to any additional properties in the instance object/array that are not explicitly mentioned in the schema

Orderly supports expressing whether or not additional properties should be allowed, but does not allow you to specify a schema which governs these additional properties.  A trailing `*` in orderly indicates additional properties are allowed, and occurs immediately after the definition of nested schemas (the closing curly brace) for both objects:

    object {
        string name;
        string title;
    }* employee;

and arrays

    array { integer; string; }* myOpenTupleTypedArray;

### null types

The null type in JSONSchema specifies a value that must be null.  The `null` type specifier is orderly's equivalent:

    null likeAir;

As explained in the JSONSchema proposal, `null` is useful "mainly for purpose of being able use union types to define nullability".  For example:

    union {
        string [ "Sr.", "Jr.", "III" ];
        null; 
    } suffix;

### any types

"Any types" are represented in orderly using the `any` type speicifier: 

    any notes;

### unions

It is possible in JSONSchema to specify a property that may be of one of many different types.  In orderly this functionality is represented using the `union` type specifier:

    union {
        string;
        number;
    } myUnion;

A key syntactic feature to note is the supported (required?) ommission of property names where they would be meaningless.

### extensions or "extra properties"

Orderly is capable of concisely representing a subset of JSONSchema, however at times it might be desirable to be able to represent properties in JSONSchema that are not supported natively in orderly.  For this reason the backtick operators will allow you to encode a json object as part of an orderly schema.  For example to attach a description to a schema entry one might generate something like:

    string `{"description": "The name of the service"}`;

The author has full control over formatting, as whitespace is ignored:

    string `{
      "title": "Service Name",
      "description": "The name of the service",
      "ui_hints": "Use the blink tag"
    }`;

### references

*TODO*.  We'll probably use the `reference` type specifier and allow the consumer to override the value name.  for example:

    object {
        string name;
        string title;
        ref "http://json-schema.org/card" secretary;
        array {
            ref "http://json-schema.org/card";
        } reports;
    } employee;

### more complex examples

A number with a range, enumerated possible values, and a default value:

    integer{0,256} powerOfTwo[1,2,4,8,16,32,64,128,256] = 1;

An object with enumerated possible values and a default.

    object {
      string beast;
      number normalTemperature;
    } temps [ { "beast": "canine", "normalTemperature": 101.2 },
        { "beast": "human", "normalTemperature": 98.6 } ]
      = { "beast": "canine", "normalTemperature": 101.2 };

### gotchas

When you stare hard enough at the grammar of a non-trival language you usually learn quite a deal.  Sometimes what you learn can be surprising or downright confusing.  Here's a tour of the darker parts alleys of orderly:

Brackets and braces -- visually a tad confusing:

    integer{7,42} secretOfLife[7,42];

and a little bit more confusing:

    array { integer{7,42}[7,42]; } secretOfLife;

The Normative Grammar
----------------------

    orderly_schema
        unnamed_entry ';'
        unnamed_entry
    
    named_entries
        named_entry ';' named_entries
        named_entry 
        # nothing
    
    unnamed_entries
        unnamed_entry ';' unnamed_entries
        unnamed_entry 
        # nothing
    
    named_entry
        definition_prefix property_name definition_suffix
        string_prefix property_name string_suffix
    
    unnamed_entry
        definition_prefix definition_suffix
        string_prefix string_suffix
    
    definition_prefix
        'integer' optional_range
        'number' optional_range
        'boolean'
        'null'
        'any'
        # a tuple-typed array 
        'array'  '{' unnamed_entries '}' optional_additional_marker optional_range 
        # a simple-typed array (notice the '*' marker is disallowed)
        'array'  '[' unnamed_entry ']' optional_range   
        'object' '{' named_entries '}' optional_additional_marker   
        'union'  '{' unnamed_entries '}'
    
    string_prefix
        'string' optional_range
    
    string_suffix
        optional_perl_regex definition_suffix
    
    definition_suffix
        optional_enum_values optional_default_value optional_requires \
            optional_optional_marker optional_extra_properties
        # nothing
    
    csv_property_names
        property_name "," csv_property_names
        property_name
    
    optional_extra_properties
        '`' json_object '`'
        # nothing
         
    optional_requires
        '<' csv_property_names '>'
        # nothing
    
    optional_optional_marker
        '?'
        # nothing  
    
    optional_additional_marker
        '*'
        # nothing  
    
    optional_enum_values
        json_array
        # nothing  
    
    optional_default_value
        '=' json_value
        # nothing  
    
    optional_range
        '{' json_number ',' json_number '}'
        '{' json_number ',' '}'
        '{' ',' json_number '}'
        '{' ',' '}'                          # meaningless, yes.
        # nothing
        
    property_name
        json_string
        [A-Za-z_\-]+                         # (alpha & underbar & dash)
    
    optional_perl_regex # perl compatible regular expressions are supported
        '/' ([^/]|\/) '/' # a Perl 5 compatible regular expression
        #nothing
     
    ----------------------------------------
    ---------- [The JSON Grammar] ----------
    ----------------------------------------
    
    json_object
        {}
        { members } 
    members
        pair
        pair , members
    pair
        json_string : json_value
    json_array
        []
        [ elements ]
    elements
        json_value
        json_value , elements
    json_value
        json_string
        json_number
        json_object
        json_array
        'true'
        'false'
        'null'
    
    ----------------------------------------
    
    json_string
        ""
        " chars "
    chars
        char
        char chars
    char
        any-Unicode-character-
            except-quote-or-backslash-or-
            control-character
        \" #double quote (")
        \\
        \/
        \b
        \f
        \n
        \r
        \t
        \u four-hex-digits 
    json_number
        int
        int frac
        int exp
        int frac exp 
    int
        digit
        digit1-9 digits
        - digit
        - digit1-9 digits 
    frac
        . digits
    exp
        e digits
    digits
        digit
        digit digits
    e
        e
        e+
        e-
        E
        E+
        E-
