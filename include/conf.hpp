#ifndef conf_HPP
# define conf_HPP

# include <cstddef>
# include <string>
# include <vector>
# include <map>
# include <vector>
# include <set>


/*  for lexer  */
enum	e_status_token
{
	WORD,
	OPEN_BLOCK,
	CLOSE_BLOCK,
	SEMICOLON,
	EOF_FILE
};

typedef struct
{
	e_status_token	status;
	std::string		token;
	int				line;
}	tokens_t;


/*  for NetworkManager  */
typedef struct conf_http
{
	long long					recv_segment_siz;
	long long					send_segment_siz;
	long long					http_max_method_siz;
	long long					http_max_uri_siz;
	long long					http_max_header_key_siz;
	long long					http_max_header_val_siz;
	long long					http_max_header_amt_siz;
	long long					http_max_body_siz;
}	conf_http_t;

/*  for parsing  */
typedef struct
{
	std::string					path;
	std::set<std::string>		limit_except;
	std::string					root;
	std::vector<std::string>	index;
	bool						autoindex;
	bool						cgi;

	/* exception */
	std::string					cgi_extension;
	std::string					cgi_path;
}	location_t;

typedef struct
{
	std::set<std::string>		name;
	int							port;
	long long					max_body_size;
	std::vector<std::string>	index;
	std::string					root;
	std::map<int, std::string>	errors;
	std::vector<location_t>		location;
}	vserver_t;



typedef std::vector<tokens_t>::iterator			tokens_it_t;
typedef std::vector<tokens_t>::const_iterator	tokens_it_c_t;
typedef std::vector<vserver_t>::iterator		vservers_it_t;
typedef std::vector<vserver_t>::const_iterator	vservers_it_c_t;

class Server_configuration
{
	public:	/* AJOUT */
		int						_line;
		std::string				_file;
		size_t					_i_file;
		std::vector<tokens_t>	_tokens;
		size_t					_i_token;
		std::vector<vserver_t>	_vservers;
		std::set<int>			_ports;
		conf_http_t				_conf_http;

	private: /* AJOUT */
		void				open_read_file(const char *path_file);
		void				lexer();
		void				parsing();

		/*  lexer fonction  */
		void				token_quote();
		void				comment();
		void				token_directive();
		void				cut_word();
		void				cut_space();

		/*  parsing fonction  */
		void				parsing_server();
		void				parse_single_value_directive(vserver_t& server);
		void				parse_multi_value_directive(vserver_t& server);
		void				parse_error_page_directive(vserver_t& server);

		void				parse_location_block(vserver_t& server);
		void				parse_single_value_directive_location(location_t& location);
		void				parse_multi_value_directive_location(location_t& location);

		void				parse_value_http();

		/*  utility functions  */
		long long			strtoll(const std::string& s, int index) 						const;
		int					strtoi(const std::string& s, int index) 						const;
		bool				verif_location_path(vserver_t& server, const std::string& path)	const;
		bool				verif_server_conflict(vserver_t& server);
		void				init_vserver(vserver_t& server);
		int					current_line(int index) const;
		const std::string	msg_error(const std::string& error, int index)					const;
		bool				is_end_token(size_t index)										const;
		bool				is_token(size_t index, const std::string& value) 				const;
		bool				has_status(size_t index, e_status_token status)					const;
		const std::string&	get_token(size_t index)											const;
		char				current_char()													const;
		void				init_conf_http();
		void				fill_ports();

	public:
		Server_configuration(const char *path_file);
		~Server_configuration();

		void	print_conf() const;

		const std::vector<vserver_t>&	get_list_vservers()	const;
		const std::set<int>&			get_list_port()		const;
		const conf_http_t&				get_conf_http()		const;
};

#endif
