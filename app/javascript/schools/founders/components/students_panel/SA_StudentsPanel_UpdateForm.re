[@bs.config {jsx: 3}];

open StudentsPanel__Types;

type teamCoachlist = (string, string, bool);

type state = {
  name: string,
  teamName: string,
  tagsToApply: list(string),
  exited: bool,
  teamCoaches: list(teamCoachlist),
  excludedFromLeaderboard: bool,
  title: string,
  affiliation: string,
  saving: bool,
  accessEndsAt: option(Js.Date.t),
};

type action =
  | UpdateName(string)
  | UpdateTeamName(string)
  | AddTag(string)
  | RemoveTag(string)
  | UpdateExited(bool)
  | UpdateCoachesList(string, string, bool)
  | UpdateExcludedFromLeaderboard(bool)
  | UpdateTitle(string)
  | UpdateAffiliation(string)
  | UpdateSaving(bool)
  | UpdateAccessEndsAt(option(Js.Date.t));

let str = ReasonReact.string;

let stringInputInvalid = s => s |> String.length < 2;

let updateName = (send, name) => {
  send(UpdateName(name));
};

let updateTeamName = (send, teamName) => {
  send(UpdateTeamName(teamName));
};

let updateTitle = (send, title) => {
  send(UpdateTitle(title));
};

let formInvalid = state =>
  state.name
  |> stringInputInvalid
  || state.teamName
  |> stringInputInvalid
  || state.title
  |> stringInputInvalid;

let handleErrorCB = (send, ()) => send(UpdateSaving(false));

let handleResponseCB = (submitCB, state, json) => {
  let teams = json |> Json.Decode.(field("teams", list(Team.decode)));
  let students =
    json |> Json.Decode.(field("students", list(Student.decode)));

  submitCB(teams, students, state.tagsToApply);
  Notification.success("Success", "Student updated successfully");
};

let updateStudent = (student, state, send, authenticityToken, responseCB) => {
  send(UpdateSaving(true));
  let payload = Js.Dict.empty();
  Js.Dict.set(
    payload,
    "authenticity_token",
    authenticityToken |> Js.Json.string,
  );
  let enrolledCoachIds =
    state.teamCoaches
    |> List.filter(((_, _, selected)) => selected == true)
    |> List.map(((key, _, _)) => key);
  let updatedStudent =
    Student.updateInfo(
      ~exited=state.exited,
      ~excludedFromLeaderboard=state.excludedFromLeaderboard,
      ~title=state.title,
      ~affiliation=Some(state.affiliation),
      ~student,
    );
  Js.Dict.set(
    payload,
    "founder",
    Student.encode(state.name, state.teamName, updatedStudent),
  );
  Js.Dict.set(
    payload,
    "tags",
    state.tagsToApply |> Json.Encode.(list(string)),
  );
  Js.Dict.set(
    payload,
    "coach_ids",
    enrolledCoachIds |> Json.Encode.(list(string)),
  );
  let url = "/school/students/" ++ (student |> Student.id);
  Api.update(url, payload, responseCB, handleErrorCB(send));
};

let boolBtnClasses = selected => {
  let classes = "toggle-button__button";
  classes ++ (selected ? " toggle-button__button--active" : "");
};

let handleEligibleTeamCoachList =
    (schoolCoaches, courseCoachIds, teamCoachIds) => {
  let selectedTeamCoachIds = teamCoachIds |> Array.of_list;
  let allowedTeamCoaches =
    schoolCoaches
    |> List.filter(coach =>
         !(
           courseCoachIds
           |> List.exists(courseCoachId => courseCoachId == Coach.id(coach))
         )
       );
  allowedTeamCoaches
  |> List.map(coach => {
       let coachId = coach |> Coach.id;
       let selected =
         selectedTeamCoachIds
         |> Js.Array.findIndex(selectedCoachId => coachId == selectedCoachId)
         > (-1);

       (coach |> Coach.id, coach |> Coach.name, selected);
     });
};

let studentTeam = (teams, student) =>
  teams
  |> ListUtils.unsafeFind(
       team => Team.id(team) === Student.teamId(student),
       "Could not find team for student (#"
       ++ (student |> Student.id)
       ++ ") editor",
     );

let initialState =
    (student, teams, schoolCoaches, courseCoachIds, teamCoachIds) => {
  name: student |> Student.name,
  teamName: student |> studentTeam(teams) |> Team.name,
  tagsToApply: student |> Student.tags,
  exited: student |> Student.exited,
  teamCoaches:
    handleEligibleTeamCoachList(schoolCoaches, courseCoachIds, teamCoachIds),
  excludedFromLeaderboard: student |> Student.excludedFromLeaderboard,
  title: student |> Student.title,
  affiliation: student |> Student.affiliation |> OptionUtils.toString,
  saving: false,
  accessEndsAt: student |> studentTeam(teams) |> Team.accessEndsAt,
};

let reducer = (state, action) =>
  switch (action) {
  | UpdateName(name) => {...state, name}
  | UpdateTeamName(teamName) => {...state, teamName}
  | AddTag(tag) => {...state, tagsToApply: [tag, ...state.tagsToApply]}
  | RemoveTag(tag) => {
      ...state,
      tagsToApply: state.tagsToApply |> List.filter(t => t !== tag),
    }
  | UpdateExited(exited) => {...state, exited}
  | UpdateCoachesList(key, value, selected) =>
    let oldCoach =
      state.teamCoaches |> List.filter(((item, _, _)) => item != key);
    {...state, teamCoaches: [(key, value, selected), ...oldCoach]};
  | UpdateExcludedFromLeaderboard(excludedFromLeaderboard) => {
      ...state,
      excludedFromLeaderboard,
    }
  | UpdateTitle(title) => {...state, title}
  | UpdateAffiliation(affiliation) => {...state, affiliation}
  | UpdateSaving(bool) => {...state, saving: bool}
  | UpdateAccessEndsAt(accessEndsAt) => {...state, accessEndsAt}
  };

[@react.component]
let make =
    (
      ~student,
      ~isSingleFounder,
      ~teams,
      ~studentTags,
      ~teamCoachIds,
      ~courseCoachIds,
      ~schoolCoaches,
      ~submitFormCB,
      ~authenticityToken,
    ) => {
  let (state, send) =
    React.useReducer(
      reducer,
      initialState(
        student,
        teams,
        schoolCoaches,
        courseCoachIds,
        teamCoachIds,
      ),
    );

  let multiSelectCoachEnrollmentsCB = (key, value, selected) =>
    send(UpdateCoachesList(key, value, selected));

  <div className="mx-auto bg-white">
    <div className="flex items-centre py-6 pl-16 mb-4 border-b bg-gray-100">
      {switch (student |> Student.avatarUrl) {
       | Some(avatarUrl) =>
         <img className="w-12 h-12 rounded-full mr-4" src=avatarUrl />
       | None =>
         <Avatar name={student |> Student.name} className="w-12 h-12 mr-4" />
       }}
      <div className="text-sm flex flex-col justify-center">
        <div className="text-black font-bold inline-block">
          {student |> Student.name |> str}
        </div>
        <div className="text-gray-600 inline-block">
          {student |> Student.email |> str}
        </div>
      </div>
    </div>
    <div className="max-w-2xl p-6 mx-auto">
      <div>
        <label
          className="inline-block tracking-wide text-xs font-semibold mb-2 leading-tight"
          htmlFor="name">
          {"Name" |> str}
        </label>
        <input
          value={state.name}
          onChange={event =>
            updateName(send, ReactEvent.Form.target(event)##value)
          }
          className="appearance-none block w-full bg-white border border-gray-400 rounded py-3 px-4 leading-snug focus:outline-none focus:bg-white focus:border-gray-500"
          id="name"
          type_="text"
          placeholder="Student name here"
        />
        <School__InputGroupError
          message="Name must have at least two characters"
          active={state.name |> stringInputInvalid}
        />
      </div>
      {isSingleFounder
         ? ReasonReact.null
         : <div className="mt-5">
             <label
               className="inline-block tracking-wide text-xs font-semibold mb-2 leading-tight"
               htmlFor="team_name">
               {"Team Name" |> str}
             </label>
             <input
               value={state.teamName}
               onChange={event =>
                 updateTeamName(send, ReactEvent.Form.target(event)##value)
               }
               className="appearance-none block w-full bg-white border border-gray-400 rounded py-3 px-4 leading-snug focus:outline-none focus:bg-white focus:border-gray-500"
               id="team_name"
               type_="text"
               placeholder="Team name here"
             />
             <School__InputGroupError
               message="Team Name must have at least two characters"
               active={state.teamName |> stringInputInvalid}
             />
           </div>}
      <div className="mt-5">
        <label
          className="inline-block tracking-wide text-xs font-semibold mb-2 leading-tight"
          htmlFor="title">
          {"Title" |> str}
        </label>
        <input
          value={state.title}
          onChange={event =>
            updateTitle(send, ReactEvent.Form.target(event)##value)
          }
          className="appearance-none block w-full bg-white border border-gray-400 rounded py-3 px-4 leading-snug focus:outline-none focus:bg-white focus:border-gray-500"
          id="title"
          type_="text"
          placeholder="Student, Coach, CEO, etc."
        />
        <School__InputGroupError
          message="Title must have at least two characters"
          active={state.title |> stringInputInvalid}
        />
      </div>
      <div className="mt-5">
        <label
          className="inline-block tracking-wide text-xs font-semibold mb-2 leading-tight"
          htmlFor="affiliation">
          {"Affiliation" |> str}
        </label>
        <span className="text-xs ml-1"> {"(optional)" |> str} </span>
        <input
          value={state.affiliation}
          onChange={event =>
            send(UpdateAffiliation(ReactEvent.Form.target(event)##value))
          }
          className="appearance-none block w-full bg-white border border-gray-400 rounded py-3 px-4 leading-snug focus:outline-none focus:bg-white focus:border-gray-500"
          id="affiliation"
          type_="text"
          placeholder="Acme Inc., Acme University, etc."
        />
      </div>
      <div className="mt-5">
        <div className="border-b pb-4 mb-2 mt-5 ">
          <span className="inline-block mr-1 text-xs font-semibold">
            {(isSingleFounder ? "Personal Coaches" : "Team Coaches") |> str}
          </span>
          <div className="mt-2">
            <School__SelectBox
              items={state.teamCoaches}
              selectCB=multiSelectCoachEnrollmentsCB
            />
          </div>
        </div>
      </div>
      <div className="mt-5">
        <div className="mb-2 text-xs font-semibold">
          {"Tags applied:" |> str}
        </div>
        <SA_StudentsPanel_SearchableTagList
          unselectedTags={
            studentTags
            |> List.filter(tag => !(state.tagsToApply |> List.mem(tag)))
          }
          selectedTags={state.tagsToApply}
          addTagCB={tag => send(AddTag(tag))}
          removeTagCB={tag => send(RemoveTag(tag))}
          allowNewTags=true
        />
      </div>
      <div className="mt-5">
        <div className="flex items-center flex-shrink-0">
          <label className="block tracking-wide text-xs font-semibold mr-3">
            {"Should this student be excluded from leaderboards?" |> str}
          </label>
          <div
            className="flex flex-shrink-0 rounded-lg overflow-hidden border border-gray-400">
            <button
              title="Exclude this student from the leaderboard"
              onClick={event => {
                ReactEvent.Mouse.preventDefault(event);
                send(UpdateExcludedFromLeaderboard(true));
              }}
              className={boolBtnClasses(state.excludedFromLeaderboard)}>
              {"Yes" |> str}
            </button>
            <button
              title="Include this student in the leaderboard"
              onClick={_event => {
                ReactEvent.Mouse.preventDefault(_event);
                send(UpdateExcludedFromLeaderboard(false));
              }}
              className={boolBtnClasses(!state.excludedFromLeaderboard)}>
              {"No" |> str}
            </button>
          </div>
        </div>
      </div>
      <div className="mt-5">
        <label
          className="tracking-wide text-xs font-semibold"
          htmlFor="access-ends-at-input">
          {"Access ends at" |> str}
        </label>
        <span className="ml-1 text-xs"> {"(optional)" |> str} </span>
        <HelpIcon
          className="ml-2"
          link="https://docs.pupilfirst.com/#/students?id=editing-student-details">
          {"If specified, students can't submit their work from the specified date"
           |> str}
        </HelpIcon>
        <DatePicker
          onChange={date => send(UpdateAccessEndsAt(date))}
          selected=?{state.accessEndsAt}
          id="access-ends-at-input"
        />
      </div>
      <div className="mt-5">
        <label
          className="tracking-wide text-xs font-semibold"
          htmlFor="access-ends-at-input">
          {"Dropout Student" |> str}
        </label>
        <HelpIcon
          className="ml-2"
          link="https://docs.pupilfirst.com/#/students?id=editing-student-details">
          {"If specified, students can't submit their work from the specified date"
           |> str}
        </HelpIcon>
        <div>
          <button className="btn btn-danger">
            {"Dropout Student" |> str}
          </button>
        </div>
      </div>
    </div>
    <div className="p-6 bg-gray-100 border-t">
      <div className="max-w-2xl px-6 mx-auto">
        <div
          className="flex max-w-2xl w-full justify-between items-center mx-auto">
          <div
            className="flex items-center flex-shrink-0 bg-red-100 py-1 px-1 rounded">
            <label
              className="block tracking-wide text-red-600 text-xs font-semibold mr-3">
              {"Has this student dropped out?" |> str}
            </label>
            <div
              className="flex flex-shrink-0 rounded-lg overflow-hidden border border-gray-400">
              <button
                title="Prevent this student from accessing the course"
                onClick={_event => {
                  ReactEvent.Mouse.preventDefault(_event);
                  send(UpdateExited(true));
                }}
                className={boolBtnClasses(state.exited)}>
                {"Yes" |> str}
              </button>
              <button
                title="Allow this student to access the course"
                onClick={_event => {
                  ReactEvent.Mouse.preventDefault(_event);
                  send(UpdateExited(false));
                }}
                className={boolBtnClasses(!state.exited)}>
                {"No" |> str}
              </button>
            </div>
          </div>
          <div className="w-auto">
            <button
              disabled={formInvalid(state)}
              onClick={_e =>
                updateStudent(
                  student,
                  state,
                  send,
                  authenticityToken,
                  handleResponseCB(submitFormCB, state),
                )
              }
              className="w-full btn btn-large btn-primary">
              {"Update Student" |> str}
            </button>
          </div>
        </div>
      </div>
    </div>
  </div>;
};
