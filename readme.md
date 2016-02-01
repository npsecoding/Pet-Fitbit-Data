### Setting up Git for Windows

Download the latest Git [here](http://git-scm.com/download/win)  

For "Configuring the line ending conversions", choose "Checkout as-is, commit as-is"

### Setting up Git config

Set up the following:

```sh
$ git config --global user.name "YOUR NAME"
$ git config --global user.email "YOUR EMAIL ADDRESS"
$ git config --global core.autocrlf false
```

Verify in `git-bash` using

```sh
cat ~/.gitconfig
```