## Basic commands:
- Register your name and your email.
    ```
    git config --global user.name 'Milenko'
    git config --global user.email 'passivelib@gmail.com'
    ```
- Set name for your default master branch to main.
   ```
    git config --global init.defaultBranch main
    ```
- Run init command in your folder that you want to manage by git.
    ```
    git init
    ```
- Add all changes to git.
    ```
    git add .
    ```
- Commit changes.
    ```
    git commit -m "message"
    ```
- See hystory for all files.
    ```
    git log
    ```
- Go back to previous version. Use has_code from log command. When we go to previous version HEAD pointer is pointing to the that version.
  All previous commits are there, and we can move from one to other.
    ```
    git checkout has_code
    ```
    If we want to return to recent commit, we should type:
    ```
    git checkout milenko
    ```
- Make a new branch new_branch, as a copy of current branch.
    ```
    git checkout -b new_branch
    ```
- Check which branch you are using.
    ```
    git branch
    git status
    ```
 - Move to branch new_branch. Branch that we want to move to has to exist. We are not creating a new branch here.
    ```
    git checkout new_branch
    ```
- Delete branch old_branch. First make sure you are currently in a different branch.
  ```
  git branch -D old_branch
  ``` 
- Rename your current branch.
    ```
    git branch -m main
    ```
- Remove all your commits after commit which has hash code hash_code. All files and changes that you have done will be there in the folder,
  just there are not added nor commited in git. For example if you commited big file that you don't want to be in repository, you can use
  reset to go to previous commit, remove the file from project and add/commit changes to git without problematic file. All other changes
  will be there.
   ```
    git reset has_code
    ```

## Working with github:
- Make empty repository on github. Example: https://github.com/passivelib/mcell_src.git.
- Create access token by going to Settings->Developer Settings->personal access tokens->Tokens->Generate new token.
  Give token name, Expiration=no expiration, check all items for repo and project.
  
- Connect your local folder with git repository https://github.com/passivelib/mcell_src.git.
  Alter this command origin is alias for https://github.com/passivelib/mcell_src.git.
    ```
    git remote add origin https://github.com/passivelib/mcell_src.git
    ```
- If repository already exists on github, you need only to clone it. Go to folder where you want to clone repository and run:
  ```
  git clone https://github.com/passivelib/mcell_src.git
  ```
- Commit to github repository branch milenko. If I have more branches I can commit any of them.
    ```
    git push -u origin milenko
    ```
- Download main branch localy on your computer to the branch you are currently in.
  If there are more branches on github, you can download any of them.
    ```
    git pull origin main
    ```
- If someone create pull request for branch milan. You can download it localy and test if everything is fine.
    ```
    git fetch origin
    git checkout -b milan origin/milan
    ```
- When you push or pull on github you have to give your email and for password you have to give access token.
  To save credential run:
    ```
    git config credential.helper store
    ```
## Merging:
- This creates a single commit in branch `milenko` with the final state of the branch `milan`.
   ```
    git checkout milenko
    git merge --squash milan
    git commit -m "Merged changes from milan (squashed)"
   ```
## Resolving conflicts:
Conflict will happen in the followint situation:
- You have copy of main branch in your local branch local_branch
- Someone merged his brach with main.
- You made changes at the same file and the same line where previous merge happened.
- You are trying to make a merge with remote main branch.

It is responsability of owner of local_branch to resolv conflict. Here are the steps.
- Switch to main branch.
   ```
   git checkout main
   ```
- Pull update from github main branh to local main branch.
  ```
   git pull origin main
  ```
- Switch to local_branch that has conflict with main branch.
   ```
   git checkout local_branch
   ```
- Merge main branch into local_branch.
  ```
   git merge main
   ```
- This command will give worning CONFLICT, automatic merge failed.
- Now you have to open files with merge conflict in code and to accept one of available versions,
  or to add something compleatly different on lines where conflict occured.
- Once you are done, add/commint and push changes to github. 
## Resources:
- Basic:
   ```
    https://www.youtube.com/watch?v=mJ-qvsxPHpY&t=920s
   ```
- Advance:
  ```
    https://www.youtube.com/watch?v=S7XpTAnSDL4
  ```
