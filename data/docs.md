    orderly_schema
        named_entry ';'
        named_entry
    
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
        optional_enum_values optional_default_value optional_requires optional_optional_marker optional_additional_marker optional_extra_properties
        # nothing
    
    csv_property_names
        property_name "," csv_property_names
        property_name
    
    optional_extra_properties
        '`' json_object '`'
         
    optional_requires
        '<' csv_property_name '>'
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
        [A-Za-z_\-]+ (alphanumeric & underbar & dash)
    
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
            except-"-or-\-or-
            control-character
        \"
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
