--
-- PostgreSQL database dump
--

-- Dumped from database version 17.2
-- Dumped by pg_dump version 17.2

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET transaction_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: admins; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.admins (
    id integer NOT NULL,
    steamid bigint NOT NULL,
    server_id integer,
    start_moderation timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    end_moderation timestamp without time zone
);


ALTER TABLE public.admins OWNER TO postgres;

--
-- Name: admins_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.admins_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.admins_id_seq OWNER TO postgres;

--
-- Name: admins_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.admins_id_seq OWNED BY public.admins.id;


--
-- Name: bans; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.bans (
    id integer NOT NULL,
    user_id integer,
    ban_date timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    reason text,
    server_id character varying(255),
    evidence text
);


ALTER TABLE public.bans OWNER TO postgres;

--
-- Name: bans_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.bans_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.bans_id_seq OWNER TO postgres;

--
-- Name: bans_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.bans_id_seq OWNED BY public.bans.id;


--
-- Name: checks; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.checks (
    id integer NOT NULL,
    user_id integer,
    check_date timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    server_id character varying(255) NOT NULL,
    status character varying(50),
    notes text
);


ALTER TABLE public.checks OWNER TO postgres;

--
-- Name: checks_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.checks_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.checks_id_seq OWNER TO postgres;

--
-- Name: checks_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.checks_id_seq OWNED BY public.checks.id;


--
-- Name: linked_accounts; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.linked_accounts (
    id integer NOT NULL,
    user_id integer,
    linked_steamid bigint NOT NULL,
    relation character varying(50)
);


ALTER TABLE public.linked_accounts OWNER TO postgres;

--
-- Name: linked_accounts_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.linked_accounts_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.linked_accounts_id_seq OWNER TO postgres;

--
-- Name: linked_accounts_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.linked_accounts_id_seq OWNED BY public.linked_accounts.id;


--
-- Name: moderators; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.moderators (
    id integer NOT NULL,
    steamid bigint NOT NULL,
    server_id integer,
    start_moderation timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    end_moderation timestamp without time zone
);


ALTER TABLE public.moderators OWNER TO postgres;

--
-- Name: moderators_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.moderators_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.moderators_id_seq OWNER TO postgres;

--
-- Name: moderators_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.moderators_id_seq OWNED BY public.moderators.id;


--
-- Name: servers; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.servers (
    id integer NOT NULL,
    name character varying(255),
    ip_address character varying(255)
);


ALTER TABLE public.servers OWNER TO postgres;

--
-- Name: servers_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.servers_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.servers_id_seq OWNER TO postgres;

--
-- Name: servers_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.servers_id_seq OWNED BY public.servers.id;


--
-- Name: users; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.users (
    id integer NOT NULL,
    steamid bigint NOT NULL,
    mac_address character varying(255) NOT NULL,
    created_at timestamp without time zone DEFAULT CURRENT_TIMESTAMP,
    persona_name text,
    most_recent boolean DEFAULT false
);


ALTER TABLE public.users OWNER TO postgres;

--
-- Name: users_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.users_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER SEQUENCE public.users_id_seq OWNER TO postgres;

--
-- Name: users_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: postgres
--

ALTER SEQUENCE public.users_id_seq OWNED BY public.users.id;


--
-- Name: admins id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.admins ALTER COLUMN id SET DEFAULT nextval('public.admins_id_seq'::regclass);


--
-- Name: bans id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.bans ALTER COLUMN id SET DEFAULT nextval('public.bans_id_seq'::regclass);


--
-- Name: checks id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.checks ALTER COLUMN id SET DEFAULT nextval('public.checks_id_seq'::regclass);


--
-- Name: linked_accounts id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.linked_accounts ALTER COLUMN id SET DEFAULT nextval('public.linked_accounts_id_seq'::regclass);


--
-- Name: moderators id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.moderators ALTER COLUMN id SET DEFAULT nextval('public.moderators_id_seq'::regclass);


--
-- Name: servers id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.servers ALTER COLUMN id SET DEFAULT nextval('public.servers_id_seq'::regclass);


--
-- Name: users id; Type: DEFAULT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.users ALTER COLUMN id SET DEFAULT nextval('public.users_id_seq'::regclass);


--
-- Name: admins admins_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.admins
    ADD CONSTRAINT admins_pkey PRIMARY KEY (id);


--
-- Name: admins admins_steamid_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.admins
    ADD CONSTRAINT admins_steamid_key UNIQUE (steamid);


--
-- Name: bans bans_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.bans
    ADD CONSTRAINT bans_pkey PRIMARY KEY (id);


--
-- Name: checks checks_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.checks
    ADD CONSTRAINT checks_pkey PRIMARY KEY (id);


--
-- Name: linked_accounts linked_accounts_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.linked_accounts
    ADD CONSTRAINT linked_accounts_pkey PRIMARY KEY (id);


--
-- Name: moderators moderators_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.moderators
    ADD CONSTRAINT moderators_pkey PRIMARY KEY (id);


--
-- Name: moderators moderators_steamid_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.moderators
    ADD CONSTRAINT moderators_steamid_key UNIQUE (steamid);


--
-- Name: servers servers_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.servers
    ADD CONSTRAINT servers_pkey PRIMARY KEY (id);


--
-- Name: users users_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.users
    ADD CONSTRAINT users_pkey PRIMARY KEY (id);


--
-- Name: users users_steamid_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.users
    ADD CONSTRAINT users_steamid_key UNIQUE (steamid);


--
-- Name: admins admins_server_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.admins
    ADD CONSTRAINT admins_server_id_fkey FOREIGN KEY (server_id) REFERENCES public.servers(id) ON DELETE CASCADE;


--
-- Name: bans bans_user_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.bans
    ADD CONSTRAINT bans_user_id_fkey FOREIGN KEY (user_id) REFERENCES public.users(id);


--
-- Name: checks checks_user_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.checks
    ADD CONSTRAINT checks_user_id_fkey FOREIGN KEY (user_id) REFERENCES public.users(id);


--
-- Name: linked_accounts linked_accounts_user_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.linked_accounts
    ADD CONSTRAINT linked_accounts_user_id_fkey FOREIGN KEY (user_id) REFERENCES public.users(id);


--
-- Name: moderators moderators_server_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.moderators
    ADD CONSTRAINT moderators_server_id_fkey FOREIGN KEY (server_id) REFERENCES public.servers(id) ON DELETE CASCADE;


--
-- PostgreSQL database dump complete
--

